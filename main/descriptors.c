#include "descriptors.h"

// ns_input shared between Generic and NS modes.
ns_input_s  ns_input        = {0};

uint8_t     ns_buffer[NS_HID_LEN]    = {0};
uint8_t     hid_buffer[GP_HID_LEN]    = {0};


// GameCube mode
gc_input_s  gc_input                = {0};
uint8_t     gc_buffer[GC_HID_LEN]   = {0};

// xInput mode
xi_input_s  xi_input                = {0};
uint8_t     xi_buffer[XI_HID_LEN]   = {0};