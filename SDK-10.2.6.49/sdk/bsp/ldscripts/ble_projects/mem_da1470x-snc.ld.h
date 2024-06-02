/*
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

/*
 * GCC ARM linker script to configure the DA1470x CM0+ SNC memory regions.
 */

__SNC_BASE    = 0x00000000;
__SNC_SIZE    = 64K;

__SHARED_BASE = 0x00030000;
__SHARED_SIZE = 128K;

MEMORY
{
  SNC    (rwx) : ORIGIN = __SNC_BASE,    LENGTH = __SNC_SIZE
  SHARED (rw)  : ORIGIN = __SHARED_BASE, LENGTH = __SHARED_SIZE
}
