#ifndef UTIL_COMMON_H
#define UTIL_COMMON_H

#include "adapter_includes.h"

// PUBLIC FUNCTIONS
// -----------------
#define CLAMP_0_255(value) ((value) < 0 ? 0 : ((value) > 255 ? 255 : (value)))

bool util_getbit(uint32_t bytes, uint8_t bit);

// -----------------
// -----------------

#endif