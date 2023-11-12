#ifndef GAMECUBE_H
#define GAMECUBE_H

#include "adapter_includes.h"

extern const tusb_desc_device_t gc_descriptor_dev;
extern const uint8_t gc_hid_report_descriptor[];
extern const uint8_t gc_hid_configuration_descriptor[];
extern const uint8_t gc_hid_configuration_descriptor_performance[];
extern const tinyusb_config_t gc_cfg;
extern const tinyusb_config_t gc_cfg_perf;

void gamecube_reset_data();
void gamecube_hid_report(gc_poll_response_s *poll_response, gc_origin_data_s *origin_data);

#endif