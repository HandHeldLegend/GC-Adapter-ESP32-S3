#ifndef SWPRO_H
#define SWPRO_H

#include "adapter_includes.h"

extern const tusb_desc_device_t swpro_device_descriptor;
extern const uint8_t swpro_hid_report_descriptor[];
extern const uint8_t swpro_configuration_descriptor[];
extern const tinyusb_config_t swpro_cfg;

void swpro_hid_report(gc_poll_response_s *poll_reponse, gc_origin_data_s *origin_data);

#endif