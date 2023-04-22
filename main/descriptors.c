#include "descriptors.h"

// Dinput mode
di_input_s di_input         = {0};
uint8_t    di_buffer[DI_HID_LEN]    = {0};

// ns_input shared between Generic and NS modes.
ns_input_s  ns_input        = {0};
uint8_t     ns_buffer[NS_HID_LEN]    = {0};

// GameCube mode
gc_input_s  gc_input                = {0};
uint8_t     gc_buffer[GC_HID_LEN]   = {0};

// XInput Legacy Mode
xid_input_s xid_input               = {0};
uint8_t     xid_buffer[XID_REPORT_LEN] = {0};
