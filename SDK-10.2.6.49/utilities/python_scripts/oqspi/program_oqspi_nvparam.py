#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2021 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.cli_programmer import CliProgrammer
from api.utils import SDK_ROOT
from api.script_base import run_script, ScriptArgumentsParser
from api.prepare_local_ini_file import prepare_local_ini_file
from api.create_nvparam import create_nvparam
from oqspi.program_oqspi_config import ProgramOQspiConfig, program_oqspi_config, ProductId

SECTOR_SIZE = 4096

def program_oqspi_nvparam(project_output_path, cfg=None, device_id=None, arm_gcc_path=None):
    config = ProgramOQspiConfig()
    while not config.is_valid():
        program_oqspi_config()
        config = ProgramOQspiConfig()

    if config.product_id == ProductId.DA1470x_00:
        nvparam_address = config.flash_size - SECTOR_SIZE
    else:
        raise RuntimeError("Unknown product ID")

    if not os.path.exists(project_output_path):
        raise RuntimeError("Build the desired project first")

    cli_programmer = CliProgrammer(cfg_path=cfg, prod_id=config.product_id.value, jlink_id=device_id)

    if not cfg:
        prepare_local_ini_file(cfg=cli_programmer.get_cfg(), device_id=device_id)

    project_path = os.path.join(project_output_path, '..')

    create_nvparam(project_output_path, [project_path + "/config", SDK_ROOT + "/sdk/middleware/adapters/include"],
                   arm_gcc_path)

    nvparam_bin = os.path.join(project_output_path, 'nvparam.bin')
    cli_programmer.write_oqspi(nvparam_address, nvparam_bin)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('--cfg', metavar='<cfg_path>', type=str, help='config path')
    parser.add_argument('--id', metavar='<serial_number>', type=str, help='device serial number')
    parser.add_argument('--arm_gcc_path', metavar='<arm_gcc_path>', type=str,
                        help='path to cross compiler')
    parser.add_argument('path', metavar='<project_output_path>', type=str,
                        help='folder which contains the binary you want to program')
    args = parser.parse_args()
    return args.path, args.cfg, args.id, args.arm_gcc_path


if __name__ == '__main__':
    ui.print_header('PROGRAM O-QSPI NVPARAM')
    run_script(program_oqspi_nvparam, parse_args)
    ui.print_footer('FINISHED')
