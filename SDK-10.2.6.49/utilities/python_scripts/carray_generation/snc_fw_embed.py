#!/usr/bin/env python3

#########################################################################################
# Copyright (C) 2020-2022 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

from __future__ import print_function
import sys
import time
import os
import binascii
import argparse
import struct
import re
from array import array

# USAGE:
# python snc_fw_embed <snc binary name> <header file>

def snc_fw_embed(src_fname, dst_fname, version_string):
    src_bin = src_fname + '.bin'
    if not os.path.exists(src_bin):
        print('Source binary file does not exist (' + src_bin + ')\n')
        return -1

    src_symbols = src_fname + '.symbols'
    if not os.path.exists(src_symbols):
        print('Source symbols file does not exist (' + src_symbols + ')\n')
        return -1

    print('SNC src binary file: ' + src_bin)
    print('SNC src symbols file: ' + src_symbols)
    print('SNC dst file: ' + dst_fname)

    src_fsize = os.stat(src_bin).st_size
    src_fsize_padding = (4 - (src_fsize % 4)) % 4
    src_fsize += src_fsize_padding
    print("FW image size (4-byte padded):\t%6d (0x%05X) bytes" % (src_fsize, src_fsize))

    timestamp = int(time.time())

    ###################################################################################################
    # Open destination file and write an error message. It will be overwritten later on if everything 
    # is OK
    ###################################################################################################
    dst_fd = open(dst_fname, "w")
    dst_fd.write("#error SNC FW generation failed!")
    dst_fd.close()

    ###################################################################################################
    # Extract symbol addresses
    ###################################################################################################
    src_symbols_fd = open(src_symbols, "rb")

    snc_gcc_etext                       = "__etext"
    snc_gcc_data_start                  = "__data_start__"
    snc_gcc_data_end                    = "__data_end__"

    snc_gcc_heap_limit                  = "__HeapLimit"
    snc_gcc_stack_limit                 = "__StackLimit"
    snc_gcc_stack_top                   = "__StackTop"
    snc_gcc_shared_start                = "__snc_shared_start__"
    snc_gcc_shared_end                  = "__snc_shared_end__"

    snc_shared_space_info               = "snc_shared_space_info"

    symbols_to_be_needed = [
        snc_gcc_data_end,
        snc_gcc_heap_limit,
        snc_gcc_stack_limit,
        snc_gcc_stack_top,
        snc_gcc_shared_start,
        snc_gcc_shared_end,
    ]
    symbols_needed_addr_values = [-1] * len(symbols_to_be_needed)

    symbols_to_be_extracted = [
        snc_gcc_etext,
        snc_gcc_data_start,
        snc_gcc_shared_start,
        snc_shared_space_info,
    ]
    symbols_extracted_addr_values = [-1] * len(symbols_to_be_extracted)

    for line in src_symbols_fd.readlines():
        toks = line.split()
        if len(toks) == 4:
            value, size, _, name = toks
        elif len(toks) == 3:
            value, _, name = toks
        else:
            continue

        name = name.decode("utf-8")
        value = int(value.decode("utf-8"), 16)

        if name in symbols_to_be_needed:
            idx = symbols_to_be_needed.index(name)
            symbols_needed_addr_values[idx] = value

        if name in symbols_to_be_extracted:
            idx = symbols_to_be_extracted.index(name)
            symbols_extracted_addr_values[idx] = value

    src_symbols_fd.close()

    ###################################################################################################
    # Create the file with SNC FW array and symbol addresses
    ###################################################################################################
    src_bin_fd = open(src_bin, "rb")

    packed_header = struct.pack(b'<4sL15scL', b'SNCx', src_fsize, version_string, b'0', timestamp)
    unpacked_header = struct.unpack('<LLLLLLL', packed_header)

    total_size = 0
    data_size = 0
    shared_data_size = 0

    print("");

    data_start_addr_value = symbols_extracted_addr_values[symbols_to_be_extracted.index(snc_gcc_data_start)]
    data_end_addr_value = symbols_needed_addr_values[symbols_to_be_needed.index(snc_gcc_data_end)]
    heap_limit_addr_value = symbols_needed_addr_values[symbols_to_be_needed.index(snc_gcc_heap_limit)]
    stack_limit_addr_value = symbols_needed_addr_values[symbols_to_be_needed.index(snc_gcc_stack_limit)]
    stack_top_addr_value = symbols_needed_addr_values[symbols_to_be_needed.index(snc_gcc_stack_top)]

    if data_end_addr_value != -1 and data_start_addr_value != -1:
        data_size = data_end_addr_value - data_start_addr_value
        total_size += data_size

    if heap_limit_addr_value != -1 and data_end_addr_value != -1:
        total_size += heap_limit_addr_value - data_end_addr_value
        
    if stack_top_addr_value != -1 and stack_limit_addr_value != -1:
        total_size += stack_top_addr_value - stack_limit_addr_value

    print("SNC firmware data\t\t\t\t%6d (0x%05X) bytes" % (total_size, total_size))

    shared_data_start_addr_value = symbols_needed_addr_values[symbols_to_be_needed.index(snc_gcc_shared_start)]
    shared_data_end_addr_value = symbols_needed_addr_values[symbols_to_be_needed.index(snc_gcc_shared_end)]
    if shared_data_end_addr_value != -1 and shared_data_start_addr_value != -1:
        shared_data_size = shared_data_end_addr_value - shared_data_start_addr_value
        print("SNC firmware shared data\t\t%6d (0x%05X) bytes" % (shared_data_size, shared_data_size))
        total_size += shared_data_size

    code_size = src_fsize - data_size

    print("SNC firmware code\t\t\t\t%6d (0x%05X) bytes" % (code_size, code_size))
    total_size += code_size

    print("Total SNC firmware size is\t\t%6d (0x%05X) bytes\n" % (total_size, total_size))

    if total_size != 0:
        dst_fd = open(dst_fname, "w")

        dst_fd.write('/**\n')
        dst_fd.write(' * Copyright (C) 2020-2022 Dialog Semiconductor.\n')
        dst_fd.write(' * This computer program includes Confidential, Proprietary Information\n')
        dst_fd.write(' * of Dialog Semiconductor. All Rights Reserved.\n')
        dst_fd.write(' */\n\n')
        
        dst_fd.write('#ifndef SNC_FW_EMBED_H_\n')
        dst_fd.write('#define SNC_FW_EMBED_H_\n\n')

        dst_fd.write('/** SNC firmware symbol addresses */\n')
        for name in symbols_to_be_extracted:
            idx = symbols_to_be_extracted.index(name)
            if symbols_extracted_addr_values[idx] != -1:
                if name == snc_gcc_shared_start:
                    name = "shared_space_start"
                if name == snc_shared_space_info:
                    name = "shared_space_info"
                dst_fd.write("#define %-31s ( 0x%08x )\n" % ("SNC_" + name.upper() + "_ADDRESS", symbols_extracted_addr_values[idx]))

        dst_fd.write('\n')
        dst_fd.write('/** SNC firmware shared space size */\n')
        dst_fd.write('#define %-31s ( %d )\n' % ("SNC_SHARED_SPACE_SIZE", shared_data_size))
        dst_fd.write('\n')

        dst_fd.write('/** SNC firmware code size (4-byte padded) */\n')
        dst_fd.write('#define %-31s ( %d )\n' % ("SNC_FW_CODE_SIZE", src_fsize))
        dst_fd.write('\n')

        dst_fd.write('/** SNC shared space area in shared RAM */\n')
        dst_fd.write('volatile uint32_t snc_shared_space_area[SNC_SHARED_SPACE_SIZE / 4] __attribute__((section(".snc_shared_ram_area")));\n')
        dst_fd.write('\n')

        dst_fd.write('/** SNC firmware image (Header plus Code and Padding) */\n')
        dst_fd.write('const uint32_t snc_fw_area[7 + (SNC_FW_CODE_SIZE / 4)] __attribute__((section(".snc_fw_area"))) = {\n')
        dst_fd.write('        // Header, 28 bytes\n')
        dst_fd.write("        0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x,\n" % unpacked_header)
        dst_fd.write('        // Code, ' + str(src_fsize) + ' bytes')
        fw_bytes = 0
        while True:
            packed_src_data = src_bin_fd.read(4)
            packed_src_data_len = len(packed_src_data)

            if packed_src_data_len == 0:
                break
            elif packed_src_data_len == 1:
                packed_src_data += b'\0\0\0'
            elif packed_src_data_len == 2:
                packed_src_data += b'\0\0'
            elif packed_src_data_len == 3:
                packed_src_data += b'\0'
    
            unpacked_src_data = struct.unpack('<L', packed_src_data)

            if fw_bytes % 32 == 0:
                dst_fd.write("\n        ")
            else:
                dst_fd.write(" ")

            fw_bytes += 4
            dst_fd.write("0x%08x," % unpacked_src_data)
    
        dst_fd.write('\n};\n\n')
        dst_fd.write('#endif /* SNC_FW_EMBED_H_ */\n')

        dst_fd.close()

    src_bin_fd.close()

    print('Finished!')


if __name__ == "__main__":
    src_fname = re.sub(r'\\ ', ' ', sys.argv[1])
    dst_fname = re.sub(r'\\ ', ' ', sys.argv[2])

    snc_fw_embed(src_fname, dst_fname, b'SNC v1.0.0')
