
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# SDK definitions

message("ldscript path is " ${LDSCRIPT_PATH}) 

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g -Xlinker --gc-sections -Wl,-Map,${PROJECT_NAME}.map -std=gnu++${CMAKE_CXX_STANDARD} -fno-strict-aliasing -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -std=gnu++2a -fstack-usage --specs=nano.specs -u_scanf_float")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb  -Os -fcommon -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror-implicit-function-declaration -Wall -g -Xlinker --gc-sections -Wl,-Map,${PROJECT_NAME}.map -std=gnu${CMAKE_C_STANDARD} -fno-strict-aliasing --specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb  -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g -T ${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/mem_da1470x.ld -T ${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/sections_da1470x.ld -Xlinker --gc-sections -L${LDSCRIPT_PATH} -Wl,-Map,${PROJECT_NAME}.map --specs=nosys.specs -Wl,--print-memory-usage")
set(CMAKE_ASM_FLAGS "-mcpu=${CMAKE_SYSTEM_PROCESSOR} -g -c -x assembler-with-cpp --specs=nano.specs -mthumb")