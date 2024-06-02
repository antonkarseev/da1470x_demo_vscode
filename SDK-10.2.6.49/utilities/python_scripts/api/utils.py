#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys

SDK_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../..'))


def is_win():
    
    return sys.platform.startswith('win')

def is_linux():
    return sys.platform.startswith('linux')

def is_mac():
    return sys.platform.startswith('darwin')

def normpath(path):
    os.path.normpath(path)

    if is_win():
        if not path.startswith('\"'):
            path = '\"' + path

        if not path.endswith('\"'):
            path += '\"'

    return path

