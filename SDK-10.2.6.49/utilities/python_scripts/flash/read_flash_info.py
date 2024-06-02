#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2021 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.script_base import run_script, ScriptArgumentsParser, ExecutionCanceled


def read_flash_info(cli_programmer=None, serial=None):
    cli_programmer.read_flash_info(serial_port=serial, silent=False)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)
    return cli, serial


if __name__ == '__main__':
    ui.print_header('READ FLASH INFO')
    run_script(read_flash_info, parse_args)
    ui.print_footer('FINISHED')
