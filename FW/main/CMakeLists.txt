idf_component_register(
    SRCS
    "main.c" 
    "rgb_itf.c"
    "joybus_itf.c"

    "../common/adapter.c"
    "../common/adapter_tusb.c"
    "../common/adapter_settings.c"
    "../common/adapter_defines.c"

    "../common/utilities/util_common.c"
    "../common/utilities/interval.c"
    "../common/utilities/joybus.c"
    "../common/utilities/rgb.c"

    "../common/usb/desc_bos.c"
    "../common/usb/ginput_driver.c"
    "../common/usb/xinput_driver.c"

    "../common/switch/switch_analog.c"
    "../common/switch/switch_commands.c"
    "../common/switch/switch_spi.c"

    "../common/ll/adapter_ll_esp32.c"

    "../common/devices/gcinput.c"
    "../common/devices/swpro.c"
    "../common/devices/webusb.c"
    "../common/devices/xinput.c"


    INCLUDE_DIRS 
    "../include"
    "../common/include"
    "../common/include/devices"
    "../common/include/ll"
    "../common/include/switch"
    "../common/include/usb"
    "../common/include/utilities"
    )
