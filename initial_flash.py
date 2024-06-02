#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import sys, os
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__), '../SDK-10.2.6.49/utilities/', 'python_scripts'))
from api.script_base import ProductId


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('build_configuration', type=str, help='Build configuration name')
    parser.add_argument('--prod_id', '-p', required=True, type=str, help='Device product id',
                        choices=[p.value for p in ProductId])
    args = parser.parse_args()

    os.system('python3 ../SDK-10.2.6.49/utilities/python_scripts/suota/v11/initial_flash.py '
              '{}/sg_watch.bin --prod_id {}'.format(args.build_configuration, args.prod_id))
