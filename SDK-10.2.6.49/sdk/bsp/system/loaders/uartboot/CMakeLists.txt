cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain.cmake)

set(CMAKE_SYSTEM_NAME Generic)

project(uartboot C CXX ASM)

# project definitions

set(DEVICE DA14706_00)

set(PRE_BUILD_EXTRA_DEFS "")
set(SDK_CONFIG_H ${CMAKE_CURRENT_SOURCE_DIR}/config/custom_config.h)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

# provide path to the your SDK/sdk location
# in my case root of SDK is named as BSP
set(SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)

set(BUILD_PROCESSOR 0) # Cortex M33

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/system.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdk.cmake)

add_definitions(-Ddg_configDEVICE=${DEVICE})

add_definitions(-DRELEASE_BUILD)

add_definitions(-include${SDK_CONFIG_H})

include_directories(${SDK_INCLUDES})

set(PROJECT_INCLUDES
    .
    include
    config
)

include_directories(${PROJECT_INCLUDES})

set(PROJECT_SRCS
    src/main.c
)

set(SOURCE_FILES ${SDK_SRCS} ${PROJECT_SRCS})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} ARGS -Oihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} ARGS -Obinary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${SIZE} ARGS --format=berkeley ${PROJECT_NAME}.elf)

set(SW_VERSION_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/sw_version.h)