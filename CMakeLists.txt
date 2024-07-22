cmake_minimum_required(VERSION 3.16)

project(da1470x_demo C CXX ASM)

# project definitions

set(BLE ON)
set(USB ON)
set(GPU ON)
set(DEBUG ON)
set(DEVICE DA14706_00)
set(REVISION DA1470x-00)

set(PRE_BUILD_EXTRA_DEFS "")
set(SDK_CONFIG_H ${CMAKE_CURRENT_SOURCE_DIR}/config/custom_config_oqspi.h)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

# provide path to the your SDK/sdk location
# in my case root of SDK is named as BSP
set(SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/SDK-10.2.6.49/sdk)

set(PYTHON_SCRYPTS_PATH ${SDK_PATH}/../utilities/python_scripts)

add_definitions(-Ddg_configBUILD_FOR_PROCESSOR=0)

set(BUILD_PROCESSOR 0) # Cortex M33

set(PERFORMANCE_METRICS 0)

# Display config
add_definitions(-DDEMO_RESX=390)
add_definitions(-DDEMO_RESY=390)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/system.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdk.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ldscript.cmake)

generate_ldscripts()

add_definitions(-Ddg_configDEVICE=${DEVICE})

add_definitions(-DAPP_CONFIG_H=${SDK_CONFIG_H})
add_definitions(-include${SDK_CONFIG_H})

include_directories(${SDK_INCLUDES})

set(PROJECT_INCLUDES
    .
    config

    src

    ui
    ui/demo
    ui/demo/resources
    ui/screens

    config

    gdi/inc

    lvgl
    lvgl/lvgl
    lvgl/lv_port
)

include_directories(${PROJECT_INCLUDES})

set(PROJECT_SRCS
    platform_devices.c

    src/main.c
    src/usb_cdc_vmsd.c  

    src/ble/pxp_reporter_task.c

    gdi/src/gdi.c 

    ui/demo/resources/Resources.c
    ui/demo/screens/activity_screen.c
    ui/demo/screens/compass_screen.c
    ui/demo/screens/menu_list_screen.c 
    ui/demo/screens/timer_screen.c 
    ui/demo/screens/watch_face_screen.c 
    ui/demo/init_screens.c 
    ui/demo/module.c 
    ui/CompassTask.c 
    ui/MainTask.c 

    lvgl/lv_port/lv_port_indev.c
    lvgl/lv_port/lv_port_gpu.c
    lvgl/lv_port/lv_port_disp.c

    ${LVGL_SRCS}
)

if(PERFORMANCE_METRICS)
    set(PROJECT_SRCS 
        ${PROJECT_SRCS}
        gdi/src/touch_simulation.c
        ui/UISimulationTask.c
        ui/demo/metrics.c 
    )

    add_definitions(-DPERFORMANCE_METRICS=${PERFORMANCE_METRICS})
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lvgl/lvgl)

set(SOURCE_FILES ${SDK_SRCS} ${PROJECT_SRCS})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if(BLE)
    find_library(BLE_LIB ble_stack_da1470x ${LIB_BLE_PATH})
    target_link_libraries(${PROJECT_NAME} ${BLE_LIB})

    message("BLE library location: " ${BLE_LIB})
endif()

if(USB)
    find_library(USB_LIB usb_lib_da1470x ${LIB_USB_PATH})
    target_link_libraries(${PROJECT_NAME} ${USB_LIB})
endif()

find_library(INTRINSIC_LIB iidirng ${INTRINSIC_LIB_PATH})
target_link_libraries(${PROJECT_NAME} ${INTRINSIC_LIB})

find_library(D2_DRIVER_LIB d2_driver ${D2_DRIVER_LIB_PATH})
target_link_libraries(${PROJECT_NAME} ${D2_DRIVER_LIB})

target_link_libraries(${PROJECT_NAME} lvgl)


set(LIB_USB_PATH ${SDK_PATH}/interfaces/usb/DA1470x-00-Release)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} ARGS -Oihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} ARGS -Obinary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${SIZE} ARGS --format=berkeley ${PROJECT_NAME}.elf)

set(SW_VERSION_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/sw_version.h)

# make SUOTA image
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND python3
    ARGS ${PYTHON_SCRYPTS_PATH}/suota/v11/mkimage.py -q -p ${REVISION} --sw_version ${SW_VERSION_PATH} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_suota.img
    WORKING_DIRECTORY ${PYTHON_SCRYPTS_PATH}
    COMMENT "SUOTA image prepare."
)

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND python3
    ARGS ${PYTHON_SCRYPTS_PATH}/api/create_nvparam.py ${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/config ${SDK_PATH}/middleware/adapters/include
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "nvparams create"
)

