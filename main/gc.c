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
gc_usb_phase_t         usb_phase           = GC_USB_IDLE;
usb_mode_t             active_usb_mode    = USB_MODE_GENERIC;
gc_type_t              active_gc_type     = GC_TYPE_UNKNOWN;

gc_probe_response_s    gc_probe_response   = {0};
gc_poll_response_s     gc_poll_response    = {0};
gc_origin_data_s       gc_origin_data      = {0};

volatile uint32_t   rx_timeout  = 0;
volatile uint32_t   rx_timeout_counts = 0;
volatile bool       rx_recieved     = false;
volatile uint32_t   rx_offset       = 0;
volatile bool       rx_vibrate    = false;

static void gamecube_rmt_isr(void* arg) 
{
    JB_RX_SYNC = JB_TX_STATISR;
    if (JB_TX_STATISR)
    {
        //JB_RX_SYNC      = 1;

        JB_TX_RDRST     = 1;
        JB_TX_RDRST     = 0;
        JB_TX_WRRST     = 1;
        JB_TX_WRRST     = 0;
        // Clear TX end interrupt bit
        JB_TX_CLEARISR  = 1;
    }
    // Status when RX is completed
    else if (JB_RX_STATISR)
    {
        rx_offset       = RMT.chmstatus[0].mem_waddr_ex_chm - GC_MEM_OFFSET;
        rx_timeout      = 0;
        JB_RX_MEMOWNER  = 1;
        JB_RX_RDRST     = 1;
        JB_RX_RDRST     = 0;
        JB_RX_BEGIN     = 0;
        JB_RX_SYNC      = 1;
        JB_RX_CLEARISR  = 1;
        rx_recieved = true;
    }
}

esp_err_t gamecube_rmt_init(void)
{  
    const char* TAG = "gamecube_rmt_init";

    cmd_phase = CMD_PHASE_PROBE;

    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    // RMT Peripheral System Config
    JB_RMT_FIFO     = 1;
    JB_RMT_CLKSEL   = 1;
    JB_TX_CLKEN     = 1;

    memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * GCMD_PROBE_LEN);

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
    JB_RX_CLKDIV    = 4;
    JB_RX_MEMSIZE   = 2;
    JB_RX_CARRIER   = 0;
    
    JB_RX_MEMOWNER  = 1;
    JB_RX_IDLETHRESH= JB_IDLE_TICKS;
    JB_RX_FILTEREN  = 1;
    JB_RX_FILTERTHR = 1;
    JB_RX_BEGIN     = 0;
    JB_RX_SYNC      = 1;
    JB_RX_SYNC      = 0;
    JB_RX_BEGIN     = 1;

    // Enable receipt complete interrupts
    JB_RX_ENAISR    = 1;

    gpio_set_direction(JB_P1_GPIO, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(JB_P1_GPIO, RMT_SIG_OUT0_IDX, 0, 0);
    gpio_matrix_in(JB_P1_GPIO, RMT_SIG_IN0_IDX, 0);

    rx_offset       = RMT.chmstatus[0].mem_waddr_ex_chm;
    //ESP_LOGI("SETUPPHASE", "Offset: %X", (unsigned int) rx_offset);

    esp_err_t err = rmt_isr_register(gamecube_rmt_isr, NULL, 0, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGI(esp_err_to_name(err), "%d", (unsigned int) err);
    }

    return err;
}

void gamecube_rmt_process(void)
{
    // Check which part of the cmd_phase we are in.
    switch(cmd_phase)
    {
        default:
        case CMD_PHASE_PROBE:
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
                    gc_probe_response.id_upper |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_probe_response.id_lower |= ((JB_RX_MEM[i+8].duration0 < JB_RX_MEM[i+8].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_probe_response.junk |= ((JB_RX_MEM[i+16].duration0 < JB_RX_MEM[i+16].duration1) ? 1 : 0) << (7-i);
                }

                if ((gc_probe_response.id_upper == GC_TYPE_WIRED) || (gc_probe_response.id_upper == GC_TYPE_WAVEBIRD))
                {
                    active_gc_type = gc_probe_response.id_upper;
                    cmd_phase = CMD_PHASE_ORIGIN;
                    memcpy(JB_TX_MEM, gcmd_origin_rmt, sizeof(rmt_item32_t) * GCMD_ORIGIN_LEN);
                }
            }
            break;

        case CMD_PHASE_ORIGIN:
            if (rx_offset != GC_ORIGIN_RESPONSE_LEN)
            {
                ESP_LOGI("ORIGINPHASE", "Command Phase Reponse Length Error.");
            }
            else
            {

                memset(&gc_poll_response, 0, 2);

                for (uint8_t i = 0; i < 3; i++)
                {
                    gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
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
                    gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.buttons_2 |= ((JB_RX_MEM[i+8].duration0 < JB_RX_MEM[i+8].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.stick_x |= ((JB_RX_MEM[i+16].duration0 < JB_RX_MEM[i+16].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.stick_y |= ((JB_RX_MEM[i+24].duration0 < JB_RX_MEM[i+24].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.cstick_x |= ((JB_RX_MEM[i+32].duration0 < JB_RX_MEM[i+32].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.cstick_y |= ((JB_RX_MEM[i+40].duration0 < JB_RX_MEM[i+40].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.trigger_l |= ((JB_RX_MEM2[i].duration0 < JB_RX_MEM2[i].duration1) ? 1 : 0) << (7-i);
                }

                for (uint8_t i = 0; i < 8; i++)
                {
                    gc_poll_response.trigger_r |= ((JB_RX_MEM2[i+8].duration0 < JB_RX_MEM2[i+8].duration1) ? 1 : 0) << (7-i);
                }

                gc_origin_data.data_set = true;

                // Subtract the data we got from 128. This will tell us how off we are from center.
                // A negative value is fine.
                gc_origin_data.stick_x      = (int) gc_poll_response.stick_x - 128;
                gc_origin_data.stick_y      = (int) gc_poll_response.stick_y - 128;
                gc_origin_data.cstick_x     = (int) gc_poll_response.cstick_x - 128;
                gc_origin_data.cstick_y     = (int) gc_poll_response.cstick_y - 128;
                gc_origin_data.trigger_l    = gc_poll_response.trigger_l;
                gc_origin_data.trigger_r    = gc_poll_response.trigger_r;

                memcpy(JB_TX_MEM, gcmd_poll_rmt, sizeof(rmt_item32_t) * GCMD_POLL_LEN);

                rgb_animate_to(COLOR_WHITE);

                cmd_phase = CMD_PHASE_POLL;
            }
            
        case CMD_PHASE_POLL:

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
                        gc_poll_response.buttons_1 |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.buttons_2 |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_x |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_y |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_x |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_y |= ((JB_RX_MEM[mem_idx].duration1 < JB_RX_MEM[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }

                    gc_poll_response.trigger_l |= ((JB_RX_MEM[47].duration1 < JB_RX_MEM2[1].duration0) ? 1 : 0) << 7;
                    mem_idx = 0;

                    for (uint8_t i = 1; i < 8; i++)
                    {
                        gc_poll_response.trigger_l |= ((JB_RX_MEM2[mem_idx].duration1 < JB_RX_MEM2[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.trigger_r |= ((JB_RX_MEM2[mem_idx].duration1 < JB_RX_MEM2[mem_idx+1].duration0) ? 1 : 0) << (7-i);
                        mem_idx += 1;
                    }
                }
                // Toss out junk data
                else if (!JB_RX_MEM[0].level0)
                {
                    for (uint8_t i = 3; i < 8; i++)
                    {
                        gc_poll_response.buttons_1 |= ((JB_RX_MEM[i].duration0 < JB_RX_MEM[i].duration1) ? 1 : 0) << (7-i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.buttons_2 |= ((JB_RX_MEM[i+8].duration0 < JB_RX_MEM[i+8].duration1) ? 1 : 0) << (7-i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_x |= ((JB_RX_MEM[i+16].duration0 < JB_RX_MEM[i+16].duration1) ? 1 : 0) << (7-i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.stick_y |= ((JB_RX_MEM[i+24].duration0 < JB_RX_MEM[i+24].duration1) ? 1 : 0) << (7-i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_x |= ((JB_RX_MEM[i+32].duration0 < JB_RX_MEM[i+32].duration1) ? 1 : 0) << (7-i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.cstick_y |= ((JB_RX_MEM[i+40].duration0 < JB_RX_MEM[i+40].duration1) ? 1 : 0) << (7-i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.trigger_l |= ((JB_RX_MEM2[i].duration0 < JB_RX_MEM2[i].duration1) ? 1 : 0) << (7-i);
                    }

                    for (uint8_t i = 0; i < 8; i++)
                    {
                        gc_poll_response.trigger_r |= ((JB_RX_MEM2[i+8].duration0 < JB_RX_MEM2[i+8].duration1) ? 1 : 0) << (7-i);
                    }
                }
            }

            break;
    }
}

void adapter_mode_task(void *param)
{
    bool mode_prev_store    = false;
    bool mode_fwd_store     = false;
    vTaskDelay(1000/portTICK_PERIOD_MS);

    for(;;)
    {
        if (cmd_phase == CMD_PHASE_PROBE)
        {
            uint32_t regread = REG_READ(GPIO_IN_REG) & PIN_MASK_GCP;

            // When the button has been released...
            if (mode_fwd_store && util_getbit(regread, NEXT_BUTTON))
            {
                tud_disconnect();
                adapter_settings.adapter_mode += 1;
                if (adapter_settings.adapter_mode == USB_MODE_MAX)
                {
                    adapter_settings.adapter_mode = 0x00;
                }
                rgb_animate_to(COLOR_BLACK);
                vTaskDelay(500/portTICK_PERIOD_MS);
                save_adapter_settings();
                esp_restart();
            }
            // Store button state
            mode_fwd_store = !util_getbit(regread, NEXT_BUTTON);

            // When the prev button has been released...
            if (mode_prev_store && util_getbit(regread, PREV_BUTTON))
            {
                tud_disconnect();
                if (adapter_settings.adapter_mode == 0)
                {
                    adapter_settings.adapter_mode = USB_MODE_MAX-1;
                }
                else
                {
                    adapter_settings.adapter_mode -= 1;
                }
                rgb_animate_to(COLOR_BLACK);
                vTaskDelay(500/portTICK_PERIOD_MS);
                save_adapter_settings();
                esp_restart();
            }
            // Store button state
            mode_prev_store = !util_getbit(regread, PREV_BUTTON);
        }
        vTaskDelay(8/portTICK_PERIOD_MS);
    }
}
