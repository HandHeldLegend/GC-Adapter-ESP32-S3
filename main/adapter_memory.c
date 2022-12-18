#include "adapter_memory.h"

nvs_handle_t adapter_mem_handle;
adapter_settings_s adapter_settings = {0};

void load_adapter_mode(void)
{
    const char* TAG = "load_adapter_mode";

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open storage
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &adapter_mem_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "During Adapter load settings, NVS Open failed.");
        return;
    }

    size_t required_size = 0;
    err = nvs_get_blob(adapter_mem_handle, "adp_settings", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return;

    ESP_LOGI(TAG, "Required size: %d", required_size);

    if (required_size > 0)
    {
       err = nvs_get_blob(adapter_mem_handle, "adp_settings", &adapter_settings, &required_size);
        if (err != ESP_OK)
        {   
            ESP_LOGE(TAG, "Could not load settings. 0x%08X", (unsigned int) adapter_settings.magic_num);
            return;
        }
        if (adapter_settings.magic_num != MAGIC_NUM)
        {
            ESP_LOGI(TAG, "Settings magic bytes don't match. Setting to default...");
            nvs_close(adapter_mem_handle);

            load_adapter_defaults();
            return;
        }
        else
        {
            ESP_LOGI(TAG, "Settings loaded with magic byte 0x%08X", (unsigned int) adapter_settings.magic_num);
            nvs_close(adapter_mem_handle);
        }
        
    }
    else
    {
        ESP_LOGI(TAG, "Settings unset. Set defaults...");
        // We need to set/save all the appropriate settings.
        nvs_close(adapter_mem_handle);

        load_adapter_defaults();
        return;
    }

    ESP_LOGI(TAG, "Adapter Settings initialized OK.");
    return;
}

void load_adapter_defaults(void)
{
    const adapter_settings_s default_s = {
        .adapter_mode = USB_MODE_GENERIC,
        .led_brightness = 50,
        .magic_num = MAGIC_NUM,
        .trigger_mode = 0x0000,
        .trigger_threshold_l = 0x00,
        .trigger_threshold_r = 0x00,
    };

    const char* TAG = "load_adapter_defaults";
    esp_err_t err;
    ESP_LOGI(TAG, "Setting defaults...");

    // Open
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &adapter_mem_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Error when opening NVS storage for HOJA Settings.");
        return;
    }

    // Set blob
    nvs_set_blob(adapter_mem_handle, "adp_settings", &default_s, sizeof(adapter_settings_s));
    nvs_commit(adapter_mem_handle);
    nvs_close(adapter_mem_handle);

    return;
}

void save_adapter_mode(void)
{
    const char* TAG = "save_adapter_mode";
    esp_err_t err;
    // Open
    err = nvs_open(SETTINGS_NAMESPACE, NVS_READWRITE, &adapter_mem_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "During Adapter settings save, NVS storage failed to open.");
        return;
    }

    nvs_set_blob(adapter_mem_handle, "adp_settings", &adapter_settings, sizeof(adapter_settings_s));

    nvs_commit(adapter_mem_handle);
    nvs_close(adapter_mem_handle);

    return;
}