#!/usr/bin/env python

#########################################################################################
# Copyright (C) 2016-2022 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import datetime
import getopt
import mimetypes
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

if platform.system() == 'Windows':
    ARM_NONE_EABI_GDB_PY = 'arm-none-eabi-gdb-py.exe'
    ARM_NONE_EABI_NM = 'arm-none-eabi-nm.exe'
else:
    ARM_NONE_EABI_GDB_PY = 'arm-none-eabi-gdb-py'
    ARM_NONE_EABI_NM = 'arm-none-eabi-nm'

DEVICE_DA1459X = "DA1459x"
DEVICE_DA1469X = "DA1469x"
DEVICE_DA1470X = "DA1470x"

BUILD_FOR_SNC_PROCESSOR = "BUILD_FOR_SNC_PROCESSOR"

MAIN_PROCESSOR = "main"
SNC_PROCESSOR = "snc"

portable_epoch = (datetime.datetime.now() - datetime.datetime(1970, 1, 1)).total_seconds()
timestamp = datetime.datetime.now().strftime("%Y%m%d-") + (str(portable_epoch)).split('.')[0]
debug_dumps = "debug_dumps/dump_" + timestamp
symbol_list_file = debug_dumps + "/" + "symbols_" + timestamp + ".list"
gdb_cmds_file = debug_dumps + "/" + "gdb_cmds_" + timestamp + ".gdb"
gdb_cmds_log_file = debug_dumps + "/" + "gdb_cmds_" + timestamp + ".log"
on_mem_snapshot_file = debug_dumps + "/" + "online_memory_snapshot_" + timestamp + ".ihex"

# Device specific values
ram_start = None
ram_stop = None
ram_size_in_words = None

def header_cmd(elf_file, xml_reg_file, cpu):
    target_device = get_target_device_from_xml(xml_reg_file)
    
    cmd = "target remote :2331\n"
    cmd += "symbol-file " + elf_file.replace('\\', '/') + "\n"
    cmd += "set pagination off\n"
    cmd += "set print pretty\n"
    cmd += "source gdb_custom_cmds.py\n"
    if target_device ==  DEVICE_DA1470X:
        if cpu == MAIN_PROCESSOR:
            cmd += "source gdb_mtb_custom_cmds.py\n"
        elif cpu == SNC_PROCESSOR: 
            cmd += "source gdb_mtb_da1470x_snc_custom_cmds.py\n"
    cmd += "set logging file " + gdb_cmds_log_file + "\n"
    cmd += "set logging on\n"
    return cmd

def mtb_cmd(xml_reg_file, cpu):
    target_device = get_target_device_from_xml(xml_reg_file)

    if target_device == DEVICE_DA1469X or target_device == DEVICE_DA1459X:
        cmd = "echo __MTB_START__ " + "\\n" + "\n"
        cmd += "mtb-reg \n"
        cmd += "mtb-fetch \n"
        cmd += "echo __MTB_END__ " + "\\n" + "\n"
    elif target_device == DEVICE_DA1470X:
        if cpu == MAIN_PROCESSOR:
            cmd = "echo __MTB_START__ " + "\\n" + "\n"
            cmd += "mtb-reg \n"
            cmd += "mtb-fetch \n"
            cmd += "echo __MTB_END__ " + "\\n" + "\n"
        elif cpu == SNC_PROCESSOR:
            cmd = "echo __MTB_START__ " + "\\n" + "\n"
            cmd += "mtb-reg-DA1470x-snc \n"
            cmd += "mtb-fetch-DA1470x-snc \n"
            cmd += "echo __MTB_END__ " + "\\n" + "\n"
    else:
        return ""
    return cmd


def expand_free_rtos_lists_cmd():
    expand_symbols = ["pxReadyTasksLists",
                      "xDelayedTaskList1",
                      "xDelayedTaskList2",
                      "pxDelayedTaskList",
                      "pxOverflowDelayedTaskList",
                      "xPendingReadyList",
                      "xSuspendedTaskList"
                      ]

    cmd = "echo __EXPAND_FREE_RTOS_LIST_START__ " + "\\n" + "\n"
    for symbol in expand_symbols:
        cmd += "echo __LIST_SYMBOL__ " + symbol + " " + "\\n" + "\n"
        cmd += "ignore-errors iterate-freertos-list " + symbol + "\n"

    cmd += "echo __EXPAND_FREE_RTOS_LIST_STOP__ " + "\\n" + "\n"
    return cmd


def arm_registers_cmd():
    cmd = "echo __ARM_REGISTERS_START__ " + "\\n" + "\n"
    cmd += "i r \n"
    cmd += "echo __ARM_REGISTERS_END__ " + "\\n" + "\n"
    return cmd


def hex_dump_cmd(xml_reg_file, cpu):
    cmd = "echo __HEX_DUMP_START__ " + "\\n" + "\n"
    target_device = get_target_device_from_xml(xml_reg_file)
    if target_device == DEVICE_DA1470X:
        if cpu == MAIN_PROCESSOR:
            ram0_start = 0xf000000
            ram0_stop = 0xf002000
            ram0_size_in_words = (ram0_stop - ram0_start) // 4            
            cmd += "x/" + str(ram0_size_in_words) + "x " + str(ram0_start) + "\n"
    cmd += "x/" + str(ram_size_in_words) + "x " + str(ram_start) + "\n"
    cmd += "echo __HEX_DUMP_END__ " + "\\n" + "\n"
    return cmd


def ihex_dump_cmd(ihex_file):
    cmd = "dump ihex memory " + ihex_file + " " + str(hex(ram_start)) + " " + str(
            hex(ram_stop)) + "\n"
    return cmd


def load_no_bin_dump_cmd(no_bin_file):
    cmd = "restore " + no_bin_file + "\n"
    return cmd


def load_bin_dump_cmd(bin_file):
    cmd = "restore " + bin_file + " binary " + str(hex(ram_start)) + "\n"
    return cmd


def footer_cmd():
    cmd = "set logging off\n"
    cmd += "quit\n"
    return cmd


def expand_free_rtos_heap_cmd():
    cmd = "echo __EXPAND_FREE_RTOS_HEAP_START__ " + "\\n" + "\n"
    cmd += "ignore-errors iterate-freertos-heap ucHeap" + "\n"
    cmd += "echo __EXPAND_FREE_RTOS_HEAP_STOP__ " + "\\n" + "\n"
    return cmd


def bt_cmd():
    cmd = "echo __CURRENT_TASK_BACKTRACE_START__ " + "\\n" + "\n"
    cmd += "bt" + "\n"
    cmd += "echo __CURRENT_TASK_BACKTRACE_STOP__ " + "\\n" + "\n"
    return cmd

def bt_all_tasks_cmd():
    cmd = "echo __ALL_TASKS_BACKTRACE_START__ " + "\\n" + "\n"
    cmd += "thread apply all bt" + "\n"
    cmd += "echo __ALL_TASKS_BACKTRACE_STOP__ " + "\\n" + "\n"
    return cmd

def show_free_rtos_current_task_cmd():
    cmd = "echo __CURRENT_FREE_RTOS_TASK_START__ " + "\\n" + "\n"
    cmd += "ignore-errors p pxCurrentTCB->pcTaskName" + "\n"
    cmd += "echo __CURRENT_FREE_RTOS_TASK_STOP__ " + "\\n" + "\n"
    return cmd


def symbols_cmd(file_handle_symbol_list):
    cmd = "echo __SYMBOLS_START__ " + "\\n" + "\n"
    for line in file_handle_symbol_list:
        addr = "0x" + line.split()[0]
        size = str(int("0x" + line.split()[1], 16))
        name = line.split()[3]
        if int(addr, 16) >= ram_start:
            # list is sorted, stop if we exceed RAM range
            if int(addr, 16) > ram_stop:
                break
            cmd += "echo __SYMBOL__ " + addr + " " + size + " " + name + " " + "\\n" + "\n"
            cmd += "ignore-errors p " + name + "\n"

    cmd += "echo __SYMBOLS_STOP__ " + "\\n" + "\n"
    return cmd

def patch_peripheral_registers_cmd():
    # Enable I3C clk otherwise any access to I3C regs will make the device
    # not properly functioning and disconnect the debugger.
    patch = "echo Enable I3C_ENABLE " + " \ " + "\n"
    patch += "monitor memU32 0x50020908 = 0x10000  \n"
    return patch

def peripheral_registers_cmd(xml_reg_file):
    target_device = get_target_device_from_xml(xml_reg_file)
    cmd = "echo __PERIPHERAL_REGISTERS_START__ " + "\\n" + "\n"
    if xml_reg_file is not None:

        tree = ET.parse(xml_reg_file)
        root = tree.getroot()

        if target_device == DEVICE_DA1470X:
            cmd +=  patch_peripheral_registers_cmd()

        for peripheral in root.iter('peripheral'):
            p_name = peripheral.find('name').text
            p_base = peripheral.find('baseAddress').text
            cmd += "echo __PERIPHERAL__: " + p_name + "\\n" + "\n"
            base = int(p_base, 16)
            for register in peripheral.find('registers').iter('register'):
                r_name = register.find('name').text
                r_offset = register.find('addressOffset').text
                r_size = register.find('size').text
                offset = int(r_offset, 16)
                address = hex(base + offset)
                cmd += "echo __REG__: " + r_name + " \ " + "\n"
                cmd += "monitor memU" + r_size + " " + address + "\n"

    cmd += "echo __PERIPHERAL_REGISTERS_STOP__ " + "\\n" + "\n"
    return cmd


def show_freertos_task_status_cmd():
    # The project must have its vApplicationIdleHook() modified accordingly in order to collect
    # task information. Then dg_configTRACK_OS_HEAP must be set to 1 in the custom header file.
    cmd = "echo __FREE_RTOS_TASK_STATUS_START__ " + "\\n" + "\n"
    cmd += "ignore-errors print-freertos-task-status" + "\n"
    cmd += "echo __FREE_RTOS_TASK_STATUS_STOP__ " + "\\n" + "\n"
    return cmd


def create_gdb_cmd_file(elf_file, off_mem_snapshot_file, xml_reg_file, cpu):
    file_handle_symbol_list = open(symbol_list_file, "r")
    file_handle_gdb_cmds = open(gdb_cmds_file, "w")
    is_on_line_debugging = True

    file_handle_gdb_cmds.write(header_cmd(elf_file, xml_reg_file, cpu))

    if off_mem_snapshot_file is not None:
        print("Loading RAM based on %s" % off_mem_snapshot_file)
        if is_bin_file(off_mem_snapshot_file):
            file_handle_gdb_cmds.write(load_bin_dump_cmd(off_mem_snapshot_file))
            is_on_line_debugging = False
        else:
            file_handle_gdb_cmds.write(load_no_bin_dump_cmd(off_mem_snapshot_file))
            is_on_line_debugging = False

    if is_on_line_debugging:
        file_handle_gdb_cmds.write(arm_registers_cmd() +
                                   bt_cmd() +
                                   bt_all_tasks_cmd() +
                                   ihex_dump_cmd(on_mem_snapshot_file) +
                                   symbols_cmd(file_handle_symbol_list) +
                                   peripheral_registers_cmd(xml_reg_file) +
                                   show_free_rtos_current_task_cmd() +
                                   expand_free_rtos_lists_cmd() +
                                   expand_free_rtos_heap_cmd() +
                                   show_freertos_task_status_cmd() +
                                   hex_dump_cmd(xml_reg_file, cpu) +
                                   mtb_cmd(xml_reg_file, cpu) +
                                   footer_cmd())

    else:  # Only RAM related commands should be executed
        file_handle_gdb_cmds.write(symbols_cmd(file_handle_symbol_list) +
                                   show_free_rtos_current_task_cmd() +
                                   expand_free_rtos_lists_cmd() +
                                   expand_free_rtos_heap_cmd() +
                                   show_freertos_task_status_cmd() +
                                   hex_dump_cmd(xml_reg_file, cpu) +
                                   footer_cmd())


# XXX Retrieving the device name by the xml file is not enough since an irrelevant xml might have 
# XXX been provided. Thus, ideally we need to verify that the retrieved device name matches the 
# XXX chip id as read from the device registers.
def get_target_device_from_xml(xml_reg_file):
    tree = ET.parse(xml_reg_file)
    return tree.getroot().find('name').text


def set_sram_conf(xml_reg_file, sram_start, sram_stop, argv, cpu):
    global ram_start, ram_stop, ram_size_in_words

    if xml_reg_file:
        target_device = get_target_device_from_xml(xml_reg_file)
        if target_device == DEVICE_DA1469X:
            ram_start = 0x20000000
            ram_stop = 0x20080000
        elif target_device == DEVICE_DA1459X:
            ram_start = 0x20000000
            ram_stop = 0x20018000
        elif target_device == DEVICE_DA1470X:
            if cpu == SNC_PROCESSOR:
                # SNC-CM0+ RAM space
                ram_start = 0x0000000
                ram_stop  = 0x0010000
            elif cpu == MAIN_PROCESSOR:
                # CM33 RAM space (includes shared RAM space)
                ram_start = 0x20000000
                ram_stop = 0x20180000
        else:
            ram_start = 0x7fc0000
            ram_stop = 0x7fe0000
        ram_size_in_words = (ram_stop - ram_start) // 4
    else:
        ram_start = sram_start
        ram_stop = sram_stop
        try:

            ram_size_in_words = (ram_stop - ram_start) // 4
        except:
            print("Invalid --sram_start and / or --sram_stop")
            usage(argv)
            sys.exit(3)


def usage(argv):
    print("usage: " + argv[
        0] + " -f <elf_file> -b <mem_snapshot_file> -t <toolchain_path> -x <registers_xml_file> --sram_start <> --sram_stop <>")
    print("")
    print("e.g 1 " + argv[0] + " -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin -f application.elf -x SVD.xml")
    print(
            "Dumps RAM contents, symbols are provided by the elf file, peripheral registers by the -x argument.")
    print("Toolchain path is provided by the -t argument.")
    print("")
    print("e.g 2 " + argv[
        0] + " -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin -f application.elf -b dump_memory.ihex --sram_start 0xXXXXXXXX --sram_stop 0xYYYYYYYY")
    print(
            "Dumps RAM contents, symbols are provided by the elf file, memory contents will be read using the provided ihex file and the sram memory area.")
    print("Toolchain path is provided by the -t argument.")
    print("")
    print("e.g 3 " + argv[
        0] + " -t /opt/gcc-arm-none-eabi-7-2017-q4-major/bin -f application.elf -b dump_memory.bin --sram_start 0xXXXXXXXX --sram_stop 0xYYYYYYYY")
    print(
            "Dumps RAM contents, symbols are provided by the elf file, memory contents will be read using the provided raw bin file restored at address given with --sram_start")
    print("Toolchain path is provided by the -t argument.")
    print("")


# +------------------------------------------------ OS dependency if any -----------------------------------------------+

def create_symbol_list(nm, elf_file):
    shell_cmd = "{} --print-size --numeric-sort {}".format(nm, elf_file)
    args = shlex.split(shell_cmd, posix=False if platform.system() == 'Windows' else True)
    p = subprocess.Popen(args, stdout=subprocess.PIPE, universal_newlines=True)
    output = p.communicate()[0]
    filter_regex = "^[0-9a-fA-F]+\s+[0-9a-fA-F]+\s+[^tTwWN\n]\s+.[\w\d]+"
    symbol_list_file_handle = open(symbol_list_file, 'w')
    lines = output.splitlines()
    for l in lines:
        if re.search(filter_regex, l):
            symbol_list_file_handle.write(l + '\n')

    symbol_list_file_handle.close()


def run_debugger(gdb):
    shell_cmd = '"' + gdb + '" -q --command=' + gdb_cmds_file
    if os.system(shell_cmd):
        print("Invalid gdb path")
        sys.exit(1)


def is_bin_file(file_name):
    # XXX not bullet proof but hopefully it will do the job
    mime = mimetypes.guess_type(file_name)
    if mime[0] == "application/octet-stream":
        return True
    else:
        return False


def copy_elf_to_debug_folder(elf_file):
    shutil.copy(elf_file, debug_dumps)


def copy_map_to_debug_folder(elf_file):
    l = len(elf_file)
    map_file = elf_file[: l - 3] + "map"
    shutil.copy(map_file, debug_dumps)


def create_folders():
    if not os.path.exists(debug_dumps):
        os.makedirs(debug_dumps)
        
def check_pattern_in_file(file_name, pattern):
    """ Check if any line in the file contains given pattern """
    with open(file_name, 'r') as file_obj:
        for line in file_obj:
            if re.search(pattern, line):
                return True
    return False

def main(argv):
    elf_file = None
    toolchain_path = None
    off_mem_snapshot_file = None
    xml_reg_file = None
    sram_start = None
    sram_stop = None
    cpu = None

    try:
        opts, args = getopt.getopt(argv[1:], "hf:b:t:x:", ["ram_start=",
                                                           "ram_stop="])
    except getopt.GetoptError:
        usage(argv)
        sys.exit(2)
    if len(argv[1:]) < 1:
        usage(argv)
        sys.exit(1)
    for opt, arg in opts:
        if opt == '-h':
            usage(argv)
            sys.exit(0)
        elif opt == '-f':
            elf_file = os.path.normpath(arg)
        elif opt == '-b':
            off_mem_snapshot_file = os.path.normpath(arg)
        elif opt == '-t':
            toolchain_path = os.path.normpath(arg)
        elif opt == '-x':
            xml_reg_file = os.path.normpath(arg)
        elif opt == '--sram_start':
            sram_start = arg
        elif opt == '--sram_stop':
            sram_stop = arg

    # Sanity check for elf file
    if elf_file is None:
        print("Missing elf file")
        usage(argv)
        sys.exit(2)
    if not os.path.exists(elf_file):
        print("Elf file \"{}\" does not exist".format(elf_file))
        sys.exit(2)
    
    # Sanity check for toolchain path 
    if toolchain_path is None:
        print("Missing toolchain path")
        usage(argv)
        sys.exit(2)
    if not os.path.exists(toolchain_path):
        print("Toolchain path \"{}\" does not exist".format(toolchain_path))
        sys.exit(2)
     
    # Sanity check for XML register file   
    if xml_reg_file is None:
        print("Missing XML register file")
        usage(argv)
        sys.exit(2)
    if not os.path.exists(xml_reg_file):
        print("XML register file \"{}\" does not exist".format(xml_reg_file))
        sys.exit(2)
    
    # Processor core name extraction (only for DA1470x)
    if get_target_device_from_xml(xml_reg_file) == DEVICE_DA1470X:
        # We assume that the makefile and the elf file always exist in the same directory
        makefile = os.path.join(os.path.dirname(elf_file), "makefile")
        if check_pattern_in_file(makefile, BUILD_FOR_SNC_PROCESSOR):
            cpu = SNC_PROCESSOR
        else:
            cpu = MAIN_PROCESSOR    
        
    gdb = os.path.join(toolchain_path, ARM_NONE_EABI_GDB_PY)
    if not os.path.exists(gdb):
        print("arm-none-eabi-gdb-py path \"{}\" does not exist".format(gdb))
        sys.exit(2)

    nm = os.path.join(toolchain_path, ARM_NONE_EABI_NM)
    if not os.path.exists(nm):
        print("arm-none-eabi-nm path \"{}\" does not exist".format(gdb))
        sys.exit(2)

    set_sram_conf(xml_reg_file, sram_start, sram_stop, argv, cpu)

    create_folders()
    copy_elf_to_debug_folder(elf_file)
    copy_map_to_debug_folder(elf_file)
    create_symbol_list(nm, elf_file)
    create_gdb_cmd_file(elf_file, off_mem_snapshot_file, xml_reg_file, cpu)
    run_debugger(gdb)
    print("Output in " + gdb_cmds_log_file)


if __name__ == "__main__":
    main(sys.argv[0:])
