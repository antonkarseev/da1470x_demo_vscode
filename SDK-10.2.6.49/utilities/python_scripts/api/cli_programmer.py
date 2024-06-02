#########################################################################################
# Copyright (C) 2015-2021 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import socket
import tempfile
import subprocess
import sys
from configparser import ConfigParser
from shlex import split

from api.application import Application, ApplicationError
from api.utils import SDK_ROOT, is_linux, is_win, is_mac

if is_win():
    CLI_PROGRAMMER_DEFAULT_PATH = os.path.join(SDK_ROOT, 'binaries/cli_programmer.exe')
elif is_linux():
    CLI_PROGRAMMER_DEFAULT_PATH = os.path.join(SDK_ROOT, 'binaries/cli_programmer')
elif is_mac():
    CLI_PROGRAMMER_DEFAULT_PATH = os.path.join(SDK_ROOT, 'binaries/cli_programmer_osx')
else:
    CLI_PROGRAMMER_DEFAULT_PATH = ''

jlink_gdb_server_instance = None


def start_jlink_gdb_server(cmd_str, port):
    global jlink_gdb_server_instance
    if jlink_gdb_server_instance is not None or cmd_str is None:
        # Already started or no command
        return

    # Check that something is listening on the same port
    check_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    if check_sock.connect_ex(('localhost', port)) == 0:
        raise RuntimeError("Some application is listening on port {} already!".format(port))
    check_sock.close()
    print("gdb start")
    sys.stdout.flush()
    print("cmd_str ", cmd_str)

    pr = subprocess.Popen(args=split(cmd_str), stderr=subprocess.DEVNULL,
                          stdout=subprocess.DEVNULL)

    print("gdb started")


    # Wait until JLinkGDBServer starts listening on socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    host = socket.gethostname()

    print("socket created ", host, port)

    # res = sock.connect_ex(('localhost', port))

    # while res != 0:
    #     res = sock.connect_ex(('localhost', port))
    #     pass
    
    print("socket connected")

    sock.close()
    jlink_gdb_server_instance = pr


def kill_jlink_gdb_server():
    global jlink_gdb_server_instance
    if jlink_gdb_server_instance is None:
        # Not started
        return

    jlink_gdb_server_instance.kill()
    jlink_gdb_server_instance.wait()


class CliProgrammer(Application):
    def __init__(self, cfg_path='', delete_cfg=False, init_baudrate=None, baudrate=None,
                 tx_port=None, tx_pin=None, rx_port=None, rx_pin=None, prod_id=None,
                 serial_timeout=None, host=None, gdb_port=None, gdb_cmd=None, trc_cmd=None,
                 bootloader=None, jlink_id=None, jlink_path=None, **kwargs):
        kwargs['path'] = kwargs.get('path', CLI_PROGRAMMER_DEFAULT_PATH)
        try:
            super(CliProgrammer, self).__init__(**kwargs)
        except ApplicationError as e:
            raise RuntimeError(str(e) + '\nPlease build cli_programmer and try again.')

        self.init_baudrate = init_baudrate
        self.baudrate = baudrate
        self.tx_port = tx_port
        self.tx_pin = tx_pin
        self.rx_port = rx_port
        self.rx_pin = rx_pin
        self.prod_id = prod_id
        self.serial_timeout = serial_timeout
        self.host = host
        self.gdb_cmd = gdb_cmd
        self.gdb_port = gdb_port
        self.trc_cmd = trc_cmd
        self.bootloader = bootloader
        self.jlink_id = jlink_id
        self.jlink_path = jlink_path
        self.fd = None
        self.delete_cfg = delete_cfg

        if not cfg_path:
            self.fd, self.__temp_cfg = tempfile.mkstemp()
            self.save(self.__temp_cfg)
            self.delete_cfg = True
            print("temp cfg", self.__temp_cfg)
        elif not os.path.exists(cfg_path):
            self.save(cfg_path)
            self.__temp_cfg = cfg_path
        else:
            self.__temp_cfg = cfg_path

    def save(self, cfg_path):
        cmd = []

        if self.init_baudrate is not None:
            cmd.extend(['-i', str(self.init_baudrate)])
        if self.baudrate is not None:
            cmd.extend(['-s', str(self.baudrate)])
        if self.tx_port is not None:
            cmd.extend(['--tx-port', str(self.tx_port)])
        if self.tx_pin is not None:
            cmd.extend(['--tx-pin', str(self.tx_pin)])
        if self.rx_port is not None:
            cmd.extend(['--rx-port', str(self.rx_port)])
        if self.rx_pin is not None:
            cmd.extend(['--rx-pin', str(self.rx_pin)])
        if self.prod_id is not None:
            cmd.extend(['--prod-id', self.prod_id])
        if self.serial_timeout is not None:
            cmd.extend(['-w', str(self.serial_timeout)])
        if self.host is not None:
            cmd.extend(['-r', self.host])
        if self.gdb_cmd is not None:
            cmd.extend(['--gdb-cmd', self.gdb_cmd])
        if self.gdb_port is not None:
            cmd.extend(['-p', str(self.gdb_port)])
        if self.trc_cmd is not None:
            cmd.extend(['--trc', str(self.trc_cmd)])
        if self.bootloader is not None:
            cmd.extend(['-b', str(self.bootloader)])

        cmd.extend(['--save', cfg_path])
        return super(CliProgrammer, self).run(args=cmd, silent=True)

    def get_cfg(self):
        return self.__temp_cfg

    def __get_gdb_server_conf_field(self, field):
        gdb_server_section = 'gdb server'
        config = ConfigParser()
        config.read(self.get_cfg())
        return config.get(gdb_server_section, field)

    def run(self, check_booter_load=True, serial_port=None, **kwargs):
        cmd = ['--cfg', self.__temp_cfg]

        if serial_port:
            cmd.append(str(serial_port))
        else:
            # Start JLinkGDBServer instance if needed
            print("start")
            start_jlink_gdb_server(self.__get_gdb_server_conf_field('gdb_server_path'),
                                   int(self.__get_gdb_server_conf_field('port')))

            if check_booter_load:
                cmd.append('--check-booter-load')

            cmd.append('--no-kill')
            cmd.append('gdbserver')

        kwargs['args'] = cmd + kwargs.get('args', [])
        return super(CliProgrammer, self).run(**kwargs)

    def read_flash_info(self, **kwargs):
        cmd = ['read_flash_info']
        return self.run(args=cmd, **kwargs)

    def write_qspi(self, address, bin_path, size=None, check_booter_load=True, **kwargs):
        cmd = ['write_qspi', str(address), bin_path]
        if size:
            cmd.append(str(size))
        return self.run(check_booter_load, args=cmd, **kwargs)

    def write_qspi_exec(self, image_file, **kwargs):
        cmd = ['write_qspi_exec', image_file]
        return self.run(args=cmd, **kwargs)

    def write_otp_exec(self, image_file, **kwargs):
        cmd = ['write_otp_exec', image_file]
        return self.run(args=cmd, **kwargs)

    def chip_erase_qspi(self, **kwargs):
        cmd = ['chip_erase_qspi']
        return self.run(args=cmd, **kwargs)

    def erase_qspi(self, address, size, **kwargs):
        cmd = ['erase_qspi', str(address), str(size)]
        return self.run(args=cmd, **kwargs)

    def write_oqspi(self, address, bin_path, size=None, check_booter_load=True, **kwargs):
        cmd = ['write_oqspi', str(address), bin_path]
        if size:
            cmd.append(str(size))
        return self.run(check_booter_load, args=cmd, **kwargs)

    def write_oqspi_exec(self, image_file, **kwargs):
        cmd = ['write_oqspi_exec', image_file]
        return self.run(args=cmd, **kwargs)

    def chip_erase_oqspi(self, **kwargs):
        cmd = ['chip_erase_oqspi']
        return self.run(args=cmd, **kwargs)

    def erase_oqspi(self, address, size, **kwargs):
        cmd = ['erase_oqspi', str(address), str(size)]
        return self.run(args=cmd, **kwargs)

    def read_partition_table(self, **kwargs):
        cmd = ['read_partition_table']
        return self.run(args=cmd, **kwargs)

    def read_otp(self, address, size, **kwargs):
        cmd = ['read_otp', str(address), str(size)]
        return self.run(args=cmd, **kwargs)

    def write_otp(self, address, length, *data, **kwargs):
        cmd = ['write_otp', str(address), str(length), ' '.join(map(str, data))]
        return self.run(args=cmd, **kwargs)

    def write_otp_raw_file(self, address, file, size=None, **kwargs):
        cmd = ['write_otp_raw_file', str(address), file]
        if size:
            cmd.append(str(size))
        return self.run(args=cmd, **kwargs)

    def write_key_asym(self, key_idx, key, **kwargs):
        cmd = ['write_key', 'asym', str(key_idx), key]
        return self.run(args=cmd, **kwargs)

    def write_key_sym(self, key_idx, key, **kwargs):
        cmd = ['write_key', 'sym', str(key_idx), key]
        return self.run(args=cmd, **kwargs)

    def __del__(self):
        if hasattr(self, 'delete_cfg') and self.delete_cfg:
            if hasattr(self, 'fd') and self.fd:
                os.close(self.fd)

            if hasattr(self, '_CliProgrammer__temp_cfg'):
                os.remove(self.__temp_cfg)
