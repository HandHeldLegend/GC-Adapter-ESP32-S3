# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(gc-pocket-adapter)

idf_component_get_property(tusb_lib espressif__tinyusb COMPONENT_LIB)
target_include_directories(${tusb_lib} PRIVATE "common/include/usb")

add_custom_command(TARGET ${PROJECT_NAME}
 					POST_BUILD
					COMMAND ${CMAKE_COMMAND} 
					-DTARGET_NAME=${PROJECT_NAME}
					-DTARGET_PATH=${CMAKE_CURRENT_SOURCE_DIR}
					-P ${CMAKE_SOURCE_DIR}/manifest.cmake
					COMMENT "Updating version in manifest.json"
					WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
					)
                    