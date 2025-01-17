
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# SDK definitions

message("ldscript path is " ${LDSCRIPT_PATH}) 

message("GCC version: " ${CMAKE_C_COMPILER_VERSION})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Xlinker --gc-sections -Wl,-Map,${PROJECT_NAME}.map -std=gnu++${CMAKE_CXX_STANDARD} -fno-strict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -std=gnu++2a -fstack-usage -u_scanf_float")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fcommon -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror-implicit-function-declaration -Wall -g3 -Xlinker --gc-sections -Wl,-Map,${PROJECT_NAME}.map -std=gnu${CMAKE_C_STANDARD} -fno-strict-aliasing -fstack-usage -u_scanf_float")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections --specs=nano.specs --specs=nosys.specs -Wl,--print-memory-usage -Werror -Wall -g3 -T ${CMAKE_CURRENT_BINARY_DIR}/mem.ld -T ${CMAKE_CURRENT_BINARY_DIR}/sections.ld -Xlinker --gc-sections -L${LDSCRIPT_PATH} -L${BLE_LIB_PATH} -Wl,-Map,${PROJECT_NAME}.map")
set(CMAKE_ASM_FLAGS "-mcpu=${CMAKE_SYSTEM_PROCESSOR} -g3 -c -x assembler-with-cpp --specs=nano.specs -mfloat-abi=hard -mfpu=fpv5-sp-d16 -mthumb")

if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 12)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-warn-rwx-segment -z noexecstack")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize -falign-functions=4 -u _printf_float")
