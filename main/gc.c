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

// Declare variables as empty
gc_cmd_phase_t         cmd_phase           = CMD_PHASE_PROBE;
gc_probe_response_s    gc_probe_response   = {0};
gc_poll_response_s     gc_poll_response    = {0};
gc_origin_data_s       gc_origin_data      = {0};

volatile uint32_t   rx_timeout  = 0;
volatile bool       rx_recieved     = false;
volatile uint32_t   rx_offset       = 0;

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
        rx_offset       = RMT.chmstatus[0].mem_waddr_ex_chm - GC_MEM_OFFSET;
        rx_timeout      = 0;
        JB_RX_MEMOWNER  = 0;
        JB_RX_RDRST     = 1;
        JB_RX_RDRST     = 0;
        JB_RX_BEGIN     = 0;
        JB_RX_SYNC      = 1;
        JB_RX_CLEARISR  = 1;

        rx_recieved = true;
    }
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

    rx_offset       = RMT.chmstatus[0].mem_waddr_ex_chm;
    ESP_LOGI("SETUPPHASE", "Offset: %X", (unsigned int) rx_offset);

    rmt_isr_register(gamecube_rmt_isr, NULL, 3, NULL);

    return ESP_OK;
}