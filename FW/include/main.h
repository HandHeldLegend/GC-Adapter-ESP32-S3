#ifndef MAIN_H
#define MAIN_H

#include "adapter_includes.h"

// Some definitions for USB Timing
#define TIMEOUT_GC_US 500
#define TIMEOUT_COUNTS 10

#define NON_P_MODE_DELAY 550

#define GC_AXIS_CENTER 128
#define SCALE_AXIS_CENTER 128
#define SIGNED_SCALER (float)

#define LED_BUTTON_PIN (GPIO_NUM_9)
#define LED_USB_PIN (GPIO_NUM_10)

#define LED_PIN_MASK ( (1ULL << LED_BUTTON_PIN) | (1ULL << LED_USB_PIN) )



#endif

