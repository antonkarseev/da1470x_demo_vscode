#########################################################################################
# Copyright (C) 2015-2020 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import enum

from api.application import Application, ApplicationError
from api.utils import is_linux, is_win, is_mac, SDK_ROOT, normpath

if is_win():
    MKIMAGE_BIN = os.path.join(SDK_ROOT, 'binaries/mkimage.exe')
elif is_linux():
    MKIMAGE_BIN = os.path.join(SDK_ROOT, 'binaries/mkimage')
elif is_mac():
    MKIMAGE_BIN = os.path.join(SDK_ROOT, 'binaries/mkimage_osx')
else:
    MKIMAGE_BIN = ''


@enum.unique
class EllipticCurve(enum.Enum):
    SECP192R1 = 'SECP192R1'
    SECP224R1 = 'SECP224R1'
    SECP256R1 = 'SECP256R1'
    SECP384R1 = 'SECP384R1'
    BP256R1 = 'BP256R1'
    BP384R1 = 'BP384R1'
    BP512R1 = 'BP512R1'
    SECP192K1 = 'SECP192K1'
    SECP224K1 = 'SECP224K1'
    SECP256K1 = 'SECP256K1'
    CURVE25519 = 'CURVE25519'
    EDWARDS25519 = 'EDWARDS25519'


@enum.unique
class HashMethod(enum.Enum):
    SHA224 = 'SHA-224'
    SHA256 = 'SHA-256'
    SHA384 = 'SHA-384'
    SHA512 = 'SHA-512'


class Mkimage(Application):
    def __init__(self, path=MKIMAGE_BIN):
        try:
            super(Mkimage, self).__init__(path)
        except ApplicationError as e:
            raise RuntimeError(str(e) + '\nPlease build mkimage and try again.')

    def single(self, in_file, version_file, out_file, enc=None, key=None, iv=None, **kwargs):
        cmd = ['single', normpath(in_file), normpath(version_file), normpath(out_file)]
        if enc:
            cmd.append(enc)
        if key:
            cmd.append(key)
        if iv:
            cmd.append(iv)
        return self.run(args=cmd, **kwargs)

    def gen_sym_key(self, num=None, key_len=None, **kwargs):
        cmd = ['gen_sym_key']
        if num:
            cmd.append(str(num))
        if key_len:
            cmd.append(str(key_len))
        return self.run(args=cmd, **kwargs)

    def gen_asym_key(self, ec, num=None, **kwargs):
        cmd = ['gen_asym_key', ec.value]
        if num:
            cmd.append(str(num))
        return self.run(args=cmd, **kwargs)

    def secure(self, in_file, ver_file, out_file, ec, hash_type, key, key_id, rev_string=None,
               min_ver=False, ver=None, **kwargs):
        cmd = ['secure', normpath(in_file), normpath(ver_file), normpath(out_file), ec.value,
               hash_type.value, key, str(key_id)]
        if rev_string:
            cmd.extend(['rev', '"{}"'.format(rev_string)])
        if min_ver:
            cmd.append('min_ver')
            if ver:
                cmd.append(ver)
        return self.run(args=cmd, **kwargs)

    def da1469x(self, in_file, ver_file, out_file, **kwargs):
        cmd = ['da1469x', normpath(in_file), normpath(ver_file), normpath(out_file)]
        return self.run(args=cmd, **kwargs)

    def da1469x_secure(self, in_file, ver_file, out_file, key, key_id, sym_key, sym_key_idx,
                     nonce=None, rev_string=None, **kwargs):
        cmd = ['da1469x', normpath(in_file), normpath(ver_file), normpath(out_file), key,
               str(key_id), sym_key, str(sym_key_idx)]
        if nonce:
            cmd.extend(['nonce', nonce])
        if rev_string:
            cmd.extend(['rev', '"{}"'.format(rev_string)])
        return self.run(args=cmd, **kwargs)

    def da1470x(self, in_file, ver_file, out_file, fw_version=0, **kwargs):
        cmd = ['da1470x', normpath(in_file), normpath(ver_file), normpath(out_file), str(fw_version)]
        return self.run(args=cmd, **kwargs)

    def da1470x_secure(self, in_file, ver_file, out_file, fw_version=0, key=None, key_id=0, sym_key=None, sym_key_idx=0,
                       min_fw=None, nonce=None, rev_string=None, **kwargs):
        cmd = ['da1470x', normpath(in_file), normpath(ver_file), normpath(out_file), str(fw_version), key,
               str(key_id), sym_key, str(sym_key_idx)]
        if min_fw:
            cmd.extend(['min_fw', min_fw])
        if nonce:
            cmd.extend(['nonce', nonce])
        if rev_string:
            cmd.extend(['rev', '"{}"'.format(rev_string)])
        return self.run(args=cmd, **kwargs)
