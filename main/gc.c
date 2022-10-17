#include "gc.h"

static void gamecube_rmt_isr(void* arg) 
{
    
}

void gamecube_init(void)
{
    const char* TAG = "gamecube_init";

    periph_ll_enable_clk_clear_rst(PERIPH_RMT_MODULE);

    JB_RMT_FIFO_PTR = RMT_DATA_MODE_MEM;

    // set up RMT peripherial register stuff
    // for receive channel
    JB_RX_CLKDIV_PTR        = 10; // 0.125us increments.
    JB_RX_IDLETHRESH_PTR    = 35; // 4us idle
    JB_RX_MEMSIZE_PTR       = 1;

    JB_RX_MEMOWNER_PTR      = RMT_MEM_OWNER_RX;
    JB_RX_RDRST_PTR         = 1;
    JB_RX_COMPLETEISR_PTR   = 1;

    #if CONFIG_IDF_TARGET_ESP32S3
    #elif CONFIG_IDF_TARGET_ESP32
    RMT.conf_ch[1].conf1.ref_always_on  = RMT_BASECLK_APB;
    RMT.conf_ch[0].conf1.ref_always_on  = RMT_BASECLK_APB;
    RMT.conf_ch[0].conf1.mem_owner      = RMT_MEM_OWNER_TX;
    #endif 

    // set up RMT peripheral register stuff
    // for transaction channels :)
    JB_TX_CLKDIV_PTR    = 20; // 0.25 us increments
    JB_TX_MEMSIZE_PTR   = 1;
    JB_TX_CONT_PTR      = 0;
    JB_TX_CARRIER_PTR   = 0;
    JB_TX_IDLELVL_PTR   = RMT_IDLE_LEVEL_HIGH;
    JB_TX_IDLEEN_PTR    = 1;

    // Enable transaction complete interrupts
    JB_TX_COMPLETEISR_PTR = 1;

    rmt_item32_t gcmd_probe_rmt[] = {
        JB_RMT_0X0, JB_RMT_0X9,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X3,
        JB_STOP, JB_ZERO
    };

    // Set up the canned response to origin
    rmt_item32_t gcmd_origin_rmt[] = {
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X8, JB_RMT_0X0, 
        JB_RMT_0X8, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_RMT_0X0, JB_RMT_0X0,
        JB_STOP, JB_ZERO
    };

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[JB_RMT_GPIO], PIN_FUNC_GPIO);
    gpio_set_direction(JB_RMT_GPIO, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_matrix_out(JB_RMT_GPIO, RMT_SIG_OUT0_IDX + JB_TX_CHANNEL, 0, 0);
    gpio_matrix_in(JB_RMT_GPIO, RMT_SIG_IN0_IDX + JB_RX_CHANNEL, 0);

    rmt_isr_register(gamecube_rmt_isr, NULL, 3, NULL);

}

esp_err_t gamecube_reader_start()
{  
    const char* TAG = "gamecube_reader_start";
    esp_err_t er = ESP_OK;
    ESP_LOGI(TAG, "GameCube Reader Started.");

    gamecube_init();

    vTaskDelay(200/portTICK_PERIOD_MS);

    return ESP_OK;
}