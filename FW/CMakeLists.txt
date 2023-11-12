# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

add_compile_definitions(
    CFG_TUD_VENDOR=2
    CFG_TUD_HID=2
    CFG_TUD_HID_EP_BUFSIZE=64
    CFG_TUD_VENDOR_EPSIZE=64
    CFG_TUD_VENDOR_RX_BUFSIZE=64
    CFG_TUD_VENDOR_TX_BUFSIZE=64
)

project(gc-pocket-adapter)
