#include "main.h"
#include "rmt_gc.h"

#include "driver/rmt.h"
#include "driver/rmt_types_legacy.h"

// HAL
#include "hal/rmt_types.h"
#include "hal/rmt_hal.h"
#include "hal/dma_types.h"
#include "hal/clk_gate_ll.h"
#include "hal/rmt_ll.h"
#include "soc/rmt_struct.h"

#define GCMD_PROBE_LEN 10
#define GCMD_ORIGIN_LEN 10
#define GCMD_POLL_LEN 26
#define RX_TIMEOUT_THRESH 5000

#define GC_PROBE_RESPONSE_LEN 25
#define GC_ORIGIN_RESPONSE_LEN 81
#define GC_POLL_RESPONSE_LEN 65
#define GC_MEM_OFFSET 0xC0

// The index of the RMT memory where the outgoing vibrate bit is set
#define GC_POLL_VIBRATE_IDX 23

typedef struct
{
    int lx_offset;
    int ly_offset;
    int rx_offset;
    int ry_offset;
    int lt_offset;
    int rt_offset;
} analog_offset_s;

analog_offset_s _port_offset = {0};

joybus_input_s _port_joybus[4] = {0};

int _port_phase = 0;

volatile bool _port_rumble = false;

uint8_t _active_gc_type = 0;

// Probe Command
rmt_item32_t gcmd_probe_rmt[GCMD_PROBE_LEN] = {
    JB_RMT_0X0, JB_RMT_0X0,
    JB_STOP, JB_ZERO};

// Origin Command
rmt_item32_t gcmd_origin_rmt[GCMD_ORIGIN_LEN] = {
    JB_RMT_0X4, JB_RMT_0X1,
    JB_STOP, JB_ZERO};

// Poll Command
rmt_item32_t gcmd_poll_rmt[GCMD_POLL_LEN] = {
    JB_RMT_0X4, JB_RMT_0X0,
    JB_RMT_0X0, JB_RMT_0X3,
    JB_RMT_0X0, JB_RMT_0X0,
    JB_STOP, JB_ZERO};

typedef struct
{
    uint8_t id_upper;
    uint8_t id_lower;
    uint8_t junk;
} __attribute__((packed)) gc_probe_response_s;

gc_probe_response_s gc_probe_response = {0};

typedef struct
{
    union
    {
        struct
        {
            uint8_t button_a : 1;
            uint8_t button_b : 1;
            uint8_t button_x : 1;
            uint8_t button_y : 1;
            uint8_t button_start : 1;
            uint8_t wavebird_connected : 1;
            uint8_t b1blank : 2;
        };
        uint8_t buttons_1;
    };

    union
    {
        struct
        {
            uint8_t dpad_left : 1;
            uint8_t dpad_right : 1;
            uint8_t dpad_down : 1;
            uint8_t dpad_up : 1;
            uint8_t button_z : 1;
            uint8_t button_r : 1;
            uint8_t button_l : 1;
            uint8_t b2blank : 1;
        };
        uint8_t buttons_2;
    };

    uint8_t stick_x;
    uint8_t stick_y;
    uint8_t cstick_x;
    uint8_t cstick_y;
    uint8_t trigger_l;
    uint8_t trigger_r;
} __attribute__((packed)) gc_poll_response_s;

gc_poll_response_s gc_poll_response = {0};

volatile uint32_t rx_timeout = 0;
volatile uint32_t rx_timeout_counts = 0;
volatile bool rx_recieved = false;
volatile uint32_t rx_offset = 0;

float analog_scaler_f = 1.28f;

void _rmt_reset()
{
    if (_port_phase == 2)
    {
        /*
        if (active_gc_type == GC_TYPE_WIRED)
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = (rx_vibrate == true) ? JB_HIGH : JB_LOW;
        }
        else if (active_gc_type == GC_TYPE_WAVEBIRD)
        {
            JB_TX_MEM[GC_POLL_VIBRATE_IDX] = JB_LOW;
        }
        */
    }

    JB_RX_MEMOWNER = 1;

    JB_RX_RDRST = 1;
    JB_RX_RDRST = 0;
    JB_RX_CLEARISR = 1;
    JB_RX_BEGIN = 0;
    JB_RX_SYNC = 1;
    JB_RX_SYNC = 0;
    JB_RX_BEGIN = 1;
    JB_TX_RDRST = 1;
    JB_TX_WRRST = 1;

    JB_TX_CLEARISR = 1;
}

void _rmt_begin()
{
    // Start RMT transaction
    // Set mem owner
    JB_RX_MEMOWNER = 1;
    // Set RX to begin so it starts when sync bit is set.
    JB_RX_BEGIN = 1;
    // Start next transaction.
    JB_TX_BEGIN = 1;
}

void joybus_itf_enable_rumble(uint8_t interface, bool enable)
{
    _port_rumble = enable && (!interface);
}

static void _joybus_rmt_isr(void *arg)
{
    JB_RX_SYNC = JB_TX_STATISR;
    if (JB_TX_STATISR)
    {
        // JB_RX_SYNC      = 1;

        JB_TX_RDRST = 1;
        JB_TX_RDRST = 0;
        JB_TX_WRRST = 1;
        JB_TX_WRRST = 0;
        // Clear TX end interrupt bit
        JB_TX_CLEARISR = 1;
    }
    // Status when RX is completed
    else if (JB_RX_STATISR)
    {
        rx_offset = RMT.chmstatus[0].mem_waddr_ex_chm - GC_MEM_OFFSET;
        rx_timeout = 0;
        JB_RX_MEMOWNER = 1;
        JB_RX_RDRST = 1;
        JB_RX_RDRST = 0;
        JB_RX_BEGIN = 0;
        JB_RX_SYNC = 1;
        JB_RX_CLEARISR = 1;
        rx_recieved = true;
    }
}

void joybus_itf_init()
{
    const char *TAG = "joybus_itf_init";

    _port_phase = 0;
    _port_joybus[0].port_itf = -1;
    _port_joybus[1].port_itf = -1;
    _port_joybus[2].port_itf = -1;
    _port_joybus[3].port_itf = -1;

    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    // RMT Peripheral System Config
    JB_RMT_FIFO = 1;
    JB_RMT_CLKSEL = 1;
    JB_TX_CLKEN = 1;

    memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GCMD_PROBE_LEN);

    // RMT Peripheral TX Config
    JB_TX_CARRIER = 0;
    JB_TX_CARRIER2 = 0;

    JB_TX_CLKDIV = 10; // 0.25 us increments
    JB_TX_MEMSIZE = 1;
    JB_TX_CONT = 0;
    JB_TX_IDLELVL = 1;
    JB_TX_IDLEEN = 1;
    JB_TX_SYNC = 1;

    // Enable transaction complete interrupts
    JB_TX_ENAISR = 1;

    // RMT Peripheral RX Config
    JB_RX_CLKDIV = 4;
    JB_RX_MEMSIZE = 2;
    JB_RX_CARRIER = 0;

    JB_RX_MEMOWNER = 1;
    JB_RX_IDLETHRESH = JB_IDLE_TICKS;
    JB_RX_FILTEREN = 1;
    JB_RX_FILTERTHR = 1;
    JB_RX_BEGIN = 0;
    JB_RX_SYNC = 1;
    JB_RX_SYNC = 0;
    JB_RX_BEGIN = 1;

    // Enable receipt complete interrupts
    JB_RX_ENAISR = 1;

    gpio_set_direction(JB_P1_GPIO, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(JB_P1_GPIO, RMT_SIG_OUT0_IDX, 0, 0);
    gpio_matrix_in(JB_P1_GPIO, RMT_SIG_IN0_IDX, 0);

    rx_offset = RMT.chmstatus[0].mem_waddr_ex_chm;
    // ESP_LOGI("SETUPPHASE", "Offset: %X", (unsigned int) rx_offset);

    esp_err_t err = rmt_isr_register(_joybus_rmt_isr, NULL, 0, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGI(esp_err_to_name(err), "%d", (unsigned int)err);
    }
}

#define GC_ORIGIN_ADJUST 128
void _joybus_rmt_process(void)
{
    // Check which part of the cmd_phase we are in.
    switch (_port_phase)
    {
        default:
        case 0:
        {

            if (rx_offset != GC_PROBE_RESPONSE_LEN)
            {
                ESP_LOGI("CMDPHASE", "Command Phase Reponse Length Error.");
            }
            else
            {
                ESP_LOGI("CMDPHASE", "Command Phase got OK.");
                // Clear our probe response
                memset(&gc_probe_response, 0, sizeof(gc_probe_response_s));

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_probe_response.id_upper |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_probe_response.id_lower |= ((JB_RX_MEM[i + 8].duration0 < JB_RX_MEM[i + 8].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_probe_response.junk |= ((JB_RX_MEM[i + 16].duration0 < JB_RX_MEM[i + 16].duration1) ? 1 : 0) << (7 - i);
                }

                // 0x09 - wired || 0xE9 - wavebird
                if ((gc_probe_response.id_upper == 0x09) || (gc_probe_response.id_upper == 0xE9))
                {
                    _active_gc_type = gc_probe_response.id_upper;
                    _port_phase = 1;
                    memcpy(JB_TX_MEM, gcmd_origin_rmt, sizeof(rmt_item32_t) * GCMD_ORIGIN_LEN);
                }
            }
        }
        break;

        case 1:
        {
            if (rx_offset != GC_ORIGIN_RESPONSE_LEN)
            {
                ESP_LOGI("ORIGINPHASE", "Command Phase Reponse Length Error.");
            }
            else
            {

                memset(&gc_poll_response, 0, 2);

                for (uint8_t i = 0; i < 3; i++)
                {
                    gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7 - i);
                }

                // Toss out junk data
                if (gc_poll_response.b1blank != 0x00)
                {
                    break;
                }

                memset(&gc_poll_response, 0, sizeof(gc_poll_response));

                ESP_LOGI("ORIGINPHASE", "Got Origin Response OK.");

                for (uint8_t i = 3; i < 8; i++)
                {
                    gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.buttons_2 |= ((JB_RX_MEM[i + 8].duration0 < JB_RX_MEM[i + 8].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.stick_x |= ((JB_RX_MEM[i + 16].duration0 < JB_RX_MEM[i + 16].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.stick_y |= ((JB_RX_MEM[i + 24].duration0 < JB_RX_MEM[i + 24].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.cstick_x |= ((JB_RX_MEM[i + 32].duration0 < JB_RX_MEM[i + 32].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.cstick_y |= ((JB_RX_MEM[i + 40].duration0 < JB_RX_MEM[i + 40].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.trigger_l |= ((JB_RX_MEM2[i].duration0 < JB_RX_MEM2[i].duration1) ? 1 : 0) << (7 - i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.trigger_r |= ((JB_RX_MEM2[i + 8].duration0 < JB_RX_MEM2[i + 8].duration1) ? 1 : 0) << (7 - i);
                }

                // Subtract the data we got with 128. This will tell us how off we are from center.
                // A negative value is fine.
                _port_offset.lx_offset = GC_ORIGIN_ADJUST - (int)gc_poll_response.stick_x;
                _port_offset.ly_offset = GC_ORIGIN_ADJUST - (int)gc_poll_response.stick_y;
                _port_offset.rx_offset = GC_ORIGIN_ADJUST - (int)gc_poll_response.cstick_x;
                _port_offset.ry_offset = GC_ORIGIN_ADJUST - (int)gc_poll_response.cstick_y;
                _port_offset.lt_offset = -(int)gc_poll_response.trigger_l;
                _port_offset.rt_offset = -(int)gc_poll_response.trigger_r;

                memcpy(JB_TX_MEM, gcmd_poll_rmt, sizeof(rmt_item32_t) * GCMD_POLL_LEN);

                _port_phase = 2;
                _port_joybus[0].port_itf = 0;
            }
        }
        break;

        case 2:
        {
            uint8_t tmp_junk = 0x00;
            bool wavebird_connected = false;
            tmp_junk |= ((JB_RX_MEM[0].duration0 < JB_RX_MEM[0].duration1) ? 1 : 0) << (7);
            tmp_junk |= ((JB_RX_MEM[1].duration0 < JB_RX_MEM[1].duration1) ? 1 : 0) << (6);
            wavebird_connected |= ((JB_RX_MEM[2].duration0 < JB_RX_MEM[2].duration1) ? 1 : 0) << (5);

            if (rx_offset == GC_POLL_RESPONSE_LEN)
            {
                rx_offset = 0;
                memset(&gc_poll_response, 0, sizeof(gc_poll_response));

                // Handle edge case where Smash Box responds too quickly, dropping a bit.
                // In this edge case the level would start as HIGH instead of low.
                if (JB_RX_MEM[0].level0)
                {
                    uint8_t mem_idx = 2;

                    for (uint8_t i = 3; i < 8; i++)
                    {
                        gc_poll_response.buttons_1 |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.buttons_2 |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_x |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_y |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_x |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_y |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }

                    gc_poll_response.trigger_l |= ((JB_RX_MEM[47].duration1 < JB_RX_MEM2[1].duration0) ? 1 : 0) << 7;
                    mem_idx = 0;

                    for (uint8_t i = 1; i < 8; i++)
                    {
                        gc_poll_response.trigger_l |= ((JB_RX_MEM2[mem_idx].duration1 < JB_RX_MEM2[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.trigger_r |= ((JB_RX_MEM2[mem_idx].duration1 < JB_RX_MEM2[mem_idx + 1].duration0) ? 1 : 0) << (7 - i);
                        mem_idx += 1;
                    }
                }
                // Toss out junk data
                else if (!JB_RX_MEM[0].level0)
                {
                    for (uint8_t i = 3; i < 8; i++)
                    {
                        gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7 - i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.buttons_2 |= ((JB_RX_MEM[i + 8].duration0 < JB_RX_MEM[i + 8].duration1) ? 1 : 0) << (7 - i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_x |= ((JB_RX_MEM[i + 16].duration0 < JB_RX_MEM[i + 16].duration1) ? 1 : 0) << (7 - i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_y |= ((JB_RX_MEM[i + 24].duration0 < JB_RX_MEM[i + 24].duration1) ? 1 : 0) << (7 - i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_x |= ((JB_RX_MEM[i + 32].duration0 < JB_RX_MEM[i + 32].duration1) ? 1 : 0) << (7 - i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_y |= ((JB_RX_MEM[i + 40].duration0 < JB_RX_MEM[i + 40].duration1) ? 1 : 0) << (7 - i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.trigger_l |= ((JB_RX_MEM2[i].duration0 < JB_RX_MEM2[i].duration1) ? 1 : 0) << (7 - i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.trigger_r |= ((JB_RX_MEM2[i + 8].duration0 < JB_RX_MEM2[i + 8].duration1) ? 1 : 0) << (7 - i);
                    }

                    // Translate input and output accordingly
                    int lx = CLAMP_0_255(gc_poll_response.stick_x + _port_offset.lx_offset);
                    int ly = CLAMP_0_255(gc_poll_response.stick_y + _port_offset.ly_offset);
                    int rx = CLAMP_0_255(gc_poll_response.cstick_x + _port_offset.rx_offset);
                    int ry = CLAMP_0_255(gc_poll_response.cstick_y + _port_offset.ry_offset);

                    int lt = CLAMP_0_255(gc_poll_response.trigger_l + _port_offset.lt_offset);
                    int rt = CLAMP_0_255(gc_poll_response.trigger_r + _port_offset.rt_offset);

                    _port_joybus[0].stick_left_x = (uint8_t)lx;
                    _port_joybus[0].stick_left_y = (uint8_t)ly;
                    _port_joybus[0].stick_right_x = (uint8_t)rx;
                    _port_joybus[0].stick_right_y = (uint8_t)ry;

                    _port_joybus[0].analog_trigger_l = (uint8_t)lt;
                    _port_joybus[0].analog_trigger_r = (uint8_t)rt;

                    _port_joybus[0].button_a = gc_poll_response.button_a;
                    _port_joybus[0].button_b = gc_poll_response.button_b;
                    _port_joybus[0].button_x = gc_poll_response.button_x;
                    _port_joybus[0].button_y = gc_poll_response.button_y;

                    _port_joybus[0].dpad_down = gc_poll_response.dpad_down;
                    _port_joybus[0].dpad_left = gc_poll_response.dpad_left;
                    _port_joybus[0].dpad_right = gc_poll_response.dpad_right;
                    _port_joybus[0].dpad_up = gc_poll_response.dpad_up;

                    _port_joybus[0].button_start = gc_poll_response.button_start;

                    _port_joybus[0].button_l = gc_poll_response.button_l;
                    _port_joybus[0].button_r = gc_poll_response.button_r;

                    _port_joybus[0].button_z = gc_poll_response.button_z;
                }
            }
        }
        break;
    }
}

void _joybus_timeout_counter(bool reset)
{
    static uint8_t count = 0;

    if(reset) count = 0;

    count++;

    if(count>=25)
    {
        _port_joybus[0].port_itf = -1;
        _port_phase = 0;
        joybus_itf_init();
        count = 0;
    }
}

void joybus_itf_poll(joybus_input_s **out)
{
    *out = _port_joybus;
    
    _rmt_begin();
    ets_delay_us(500);
    
    _joybus_timeout_counter(rx_recieved);
    if (rx_recieved)
    {
        rx_recieved = false;
        _joybus_rmt_process();
    }
    _rmt_reset();
}
