#include "gc.h"

// Probe Command
rmt_item32_t gcmd_probe_rmt[GCMD_PROBE_LEN] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };


// Origin Command
rmt_item32_t gcmd_origin_rmt[GCMD_ORIGIN_LEN] = {
        JB_RMT_0X4, JB_RMT_0X1,
        JB_STOP, JB_ZERO
    };



// Poll Command
rmt_item32_t gcmd_poll_rmt[GCMD_POLL_LEN] = {
        JB_RMT_0X4, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X3,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };


gc_cmd_phase_t         cmd_phase           = CMD_PHASE_PROBE;
gc_probe_response_s    gc_probe_response   = {0};
gc_poll_response_s     gc_poll_response    = {0};
gc_origin_data_s       gc_origin_data      = {0};
ns_input_s             ns_input            = {0};

uint8_t usb_buffer[8] = {0};
volatile uint32_t rx_timeout = 0;
volatile bool rx_recieved   = false;

static void gamecube_rmt_isr(void* arg) 
{
    if (JB_TX_STATISR)
    {
        JB_RX_SYNC      = 1;

        JB_TX_RDRST     = 1;
        JB_TX_RDRST     = 0;
        JB_TX_WRRST     = 1;
        JB_TX_WRRST     = 0;
        // Clear TX end interrupt bit
        JB_TX_CLEARISR  = 1;
    }
    else if (JB_RX_STATISR)
    {
        rx_timeout = 0;
        JB_RX_MEMOWNER  = 0;
        JB_RX_BEGIN     = 0;
        JB_RX_RDRST     = 1;
        JB_RX_RDRST     = 0;
        JB_RX_SYNC      = 1;

        JB_RX_CLEARISR  = 1;

        rx_recieved = true;
    }
}

uint8_t dir_to_hat(uint8_t leftRight, uint8_t upDown)
{
    uint8_t ret = HAT_CENTER;

    if (leftRight == 2)
    {
        ret = HAT_RIGHT;
        if (upDown == 2)
        {
            ret = HAT_TOP_RIGHT;
        }
        else if (upDown == 0)
        {
            ret = HAT_BOTTOM_RIGHT;
        }
    }
    else if (leftRight == 0)
    {
        ret = HAT_LEFT;
        if (upDown == 2)
        {
            ret = HAT_TOP_LEFT;
        }
        else if (upDown == 0)
        {
            ret = HAT_BOTTOM_LEFT;
        }
    }

    else if (upDown == 2)
    {
        ret = HAT_TOP;
    }
    else if (upDown == 0)
    {
        ret = HAT_BOTTOM;
    }

    return ret;
}

void gamecube_send_usb(void)
{
    // Generate the USB Data
    ns_input.button_a = gc_poll_response.button_a;
    ns_input.button_b = gc_poll_response.button_b;
    ns_input.button_x = gc_poll_response.button_x;
    ns_input.button_y = gc_poll_response.button_y;

    uint8_t lr = 1 - gc_poll_response.button_dl + gc_poll_response.button_dr;
    uint8_t ud = 1 - gc_poll_response.button_dd + gc_poll_response.button_du;

    ns_input.dpad_hat = dir_to_hat(lr, ud);

    ns_input.button_plus = gc_poll_response.button_start;

    ns_input.trigger_r = gc_poll_response.button_z;

    ns_input.trigger_zl = gc_poll_response.button_l;
    ns_input.trigger_zr = gc_poll_response.button_r;
    
    int adj_x   = (int) gc_poll_response.stick_x + gc_origin_data.stick_x;
    int adj_y   = 256 - (int) gc_poll_response.stick_y + gc_origin_data.stick_y;
    int adj_cx  = (int) gc_poll_response.cstick_x + gc_origin_data.cstick_x;
    int adj_cy  = (int) gc_poll_response.cstick_y + gc_origin_data.cstick_y;

    ns_input.stick_left_x   = (uint8_t) adj_x;
    ns_input.stick_left_y   = (uint8_t) adj_y;
    ns_input.stick_right_x  = (uint8_t) adj_cx;
    ns_input.stick_right_y  = (uint8_t) adj_cy;

    memcpy(&usb_buffer, &ns_input, sizeof(usb_buffer));
}

esp_err_t gamecube_reader_start()
{  
    const char* TAG = "gamecube_init";

    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);
    

    // RMT Peripheral System Config
    JB_RMT_FIFO     = 1;
    JB_RMT_CLKSEL   = 1;
    JB_TX_CLKEN     = 1;

    // RMT Peripheral TX Config
    JB_TX_CARRIER   = 0;
    JB_TX_CARRIER2  = 0;
    
    JB_TX_CLKDIV    = 10; // 0.25 us increments
    JB_TX_MEMSIZE   = 1;
    JB_TX_CONT      = 0;
    JB_TX_IDLELVL   = 1;
    JB_TX_IDLEEN    = 1;
    JB_TX_SYNC      = 1;
    
    // Enable transaction complete interrupts
    JB_TX_ENAISR    = 1;

    // RMT Peripheral RX Config
    JB_RX_CLKDIV    = 10;
    JB_RX_MEMSIZE   = 2;
    JB_RX_CARRIER   = 0;
    
    JB_RX_MEMOWNER  = 1;
    JB_RX_IDLETHRESH= JB_IDLE_TICKS;
    JB_RX_FILTEREN  = 1;
    JB_RX_FILTERTHR = 1;
    JB_RX_BEGIN     = 0;
    JB_RX_SYNC      = 1;

    // Enable receipt complete interrupts
    JB_RX_ENAISR    = 1;

    gpio_set_direction(JB_P1_GPIO, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(JB_P1_GPIO, RMT_SIG_OUT0_IDX, 0, 0);
    gpio_matrix_in(JB_P1_GPIO, RMT_SIG_IN0_IDX, 0);

    memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GCMD_PROBE_LEN);

    for (int i = 0; i < 8; i++)
    {
        ESP_LOGI("TEST: ", "%X", (unsigned int) JB_TX_MEM[i].val);
    }

    cmd_phase = CMD_PHASE_PROBE;

    rmt_isr_register(gamecube_rmt_isr, NULL, 3, NULL);

    return ESP_OK;
}