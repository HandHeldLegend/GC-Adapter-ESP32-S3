#ifndef XINPUT_DEVICE_H
#define XINPUT_DEVICE_H

#include "adapter_includes.h"

void xinputd_init(void);

void xinputd_reset(uint8_t rhport);

uint16_t xinputd_open(uint8_t rhport, tusb_desc_interface_t const * desc_itf, uint16_t max_len);

bool xinputd_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request);

bool xinputd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes);

bool tud_xinput_report(void const * report, uint16_t len);
bool tud_xinput_ready(void);

extern const usbd_class_driver_t tud_xinput_driver;

uint8_t const *xinput_descriptor_device_cb(void);
uint8_t const *xinput_descriptor_configuration_cb(uint8_t index);

esp_err_t xinput_driver_install(void);

#endif