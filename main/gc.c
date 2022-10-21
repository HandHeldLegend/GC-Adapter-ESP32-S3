#include "gc.h"

#define GCMD_PROBE_LEN  10


// Probe Command
rmt_item32_t gcmd_probe_rmt[GCMD_PROBE_LEN] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };


// Origin Command
rmt_item32_t gcmd_origin_rmt[] = {
        JB_RMT_0X4, JB_RMT_0X1,
        JB_STOP, JB_ZERO
    };



// Poll Command
rmt_item32_t gcmd_poll_rmt[] = {
        JB_RMT_0X4, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X3,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };

volatile uint32_t count = 0;

static void gamecube_rmt_isr(void* arg) 
{
    if (JB_TX_STATISR)
    {
        count += 1;
        // Reset TX read pointer bit
        JB_TX_RDRST     = 1;
        JB_TX_RDRST     = 0;
        JB_TX_WRRST = 1;
        JB_TX_WRRST = 0;
        // Clear TX end interrupt bit
        JB_TX_CLEARISR  = 1;

        //JB_TX_BEGIN     = 1;
    }
}

void gamecube_init(void)
{
    const char* TAG = "gamecube_init";

    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);
    
    JB_RMT_FIFO = 1;
    JB_RMT_CLKSEL = 1;
    JB_TX_CLKEN = 1;

    JB_TX_CARRIER = 0;
    JB_TX_CARRIER2 = 1;

    // set up RMT peripheral register stuff
    // for transaction channels :)
    JB_TX_CLKDIV    = 10; // 0.25 us increments
    JB_TX_MEMSIZE   = 1;
    JB_TX_CONT      = 0;
    JB_TX_IDLELVL   = 1;
    JB_TX_IDLEEN    = 1;
    JB_TX_SYNC = 1;
    

    // Enable transaction complete interrupts
    JB_TX_ENAISR = 1;

    ESP_LOGI(TAG, "Carrier En? :   %X", (int) RMT.chnconf0[RMT_TX_CHANNEL].carrier_en_chn);

    //PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[JB_P1_GPIO], PIN_FUNC_GPIO);
    gpio_set_direction(JB_P1_GPIO, GPIO_MODE_INPUT_OUTPUT_OD);
    //gpio_matrix_in(JB_P1_GPIO, RMT_SIG_IN0_IDX + RMT_RX_CHANNEL, 0);
    gpio_matrix_out(JB_P1_GPIO, RMT_SIG_OUT0_IDX, 0, 0);


    for (uint8_t i = 0; i < 10; i++)
    {
        ESP_LOGI(TAG, "DATA TO WRITE: %X", (unsigned int) gcmd_probe_rmt[i].duration0);
    }
    
    memcpy(JB_TX_MEM, gcmd_probe_rmt, sizeof(rmt_item32_t) * 10);

    for (uint8_t i = 0; i < 10; i++)
    {
        ESP_LOGI(TAG, "DATA WRITTEN TO FIRST: %X", (unsigned int) JB_TX_MEM[i].duration0);
    }

    rmt_isr_register(gamecube_rmt_isr, NULL, 3, NULL);

}

esp_err_t gamecube_reader_start()
{  
    const char* TAG = "gamecube_reader_start";
    esp_err_t er = ESP_OK;
    ESP_LOGI(TAG, "GameCube Reader Started.");

    gamecube_init();

    vTaskDelay(200/portTICK_PERIOD_MS);

    JB_TX_RDRST = 1;
    JB_TX_RDRST = 0;
    JB_TX_WRRST = 1;
    JB_TX_WRRST = 0;
    JB_TX_BEGIN = 1;
    

    for(;;)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Sent: %d", (int) count);
    }

    return ESP_OK;
}