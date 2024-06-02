#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2020 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys
import tempfile
import struct
import binascii

# sys.path.append(os.path.join(os.path.dirname(__file__), 'SDK-10.2.6.49/utilities/', 'python_scripts'))

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)
print(PROJECT_ROOT)

from api import ui
from api.mkimage import Mkimage
from api.script_base import run_script, ScriptArgumentsParser, ExecutionCanceled
from suota.v11.mkimage import make_sw_version_file
from oqspi.program_oqspi_config import ProgramOQspiConfig, program_oqspi_config, ProductId


def program_bin_oqspi(bin_file, bin_address, cli_programmer, serial=None, check_booter_load=True):

    if bin_file is not None:
        ui.print_title("Programming image")

    cli_programmer.write_oqspi(bin_address, bin_file,
                                check_booter_load=check_booter_load,
                                serial_port=serial, silent=False)

def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()

    parser.add_argument('bin_file', type=str, nargs='?', help='path to image')
    parser.add_argument('--bin_address',  dest='bin_address', help='path to image')

    parser.add_argument('--no_check_booter_load', dest='check_booter_load', action='store_false', default=True,
                        help='Use this argument in order not to use --check-booter-load argument in writing of oqspi')
    parser.add_config_args()
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)

    return args.bin_file, args.bin_address, cli, serial, args.check_booter_load


if __name__ == '__main__':
    ui.print_header('PROGRAM OQSPI')
    run_script(program_bin_oqspi, parse_args)
    ui.print_footer('FINISHED')
