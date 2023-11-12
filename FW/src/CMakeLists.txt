idf_component_register(
    SRCS

    "drivers/xinput_device.c"

    "usb/desc_bos.c"

    "utilities/util_common.c"
    "utilities/util_usb.c"

    "switch/switch_analog.c"
    "switch/switch_commands.c"
    "switch/switch_spi.c"

    "devices/gamecube.c"
    "devices/swpro.c"
    "devices/webusb.c"
    "devices/xinput.c"

    "gc_timer.c"
    "adapter_memory.c" 

    "gc.c" 
    "main.c" 
    "rgb_animator.c" 

    INCLUDE_DIRS 
    
    "../include"
    "../include/devices"
    "../include/drivers"
    "../include/switch"
    "../include/usb"
    "../include/utilities"

    )
