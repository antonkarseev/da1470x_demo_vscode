/**
 ****************************************************************************************
 *
 * @file hw_memctrl.c
 *
 * @brief Implementation of the Memory Controller Low Level Driver.
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include "hw_memctrl.h"

void hw_memctrl_reset(void)
{
        MEMCTRL->CMI_DATA_BASE_REG = 0;
        MEMCTRL->CMI_SHARED_BASE_REG = 0;
        MEMCTRL->CMI_END_REG = 0x3FF << 10;
}

void hw_memctrl_config_cmac_region(uint32_t data_base_addr, uint32_t shared_base_addr,
                                        uint32_t end_addr)
{
        ASSERT_ERROR((data_base_addr % 4) == 0);
        ASSERT_ERROR((shared_base_addr % 1024) == 0);
        ASSERT_ERROR((end_addr & 0x3FF) == 0x3FF);

        MEMCTRL->CMI_DATA_BASE_REG = data_base_addr;
        MEMCTRL->CMI_SHARED_BASE_REG = shared_base_addr;
        MEMCTRL->CMI_END_REG = end_addr;
}


void hw_memctrl_config_master_priorities(memctrl_master_priorities_t *master_priorities,
                                                uint8_t cpuc_max_stall_cycles,
                                                uint8_t cpus_max_stall_cycles,
                                                uint8_t dma_max_stall_cycles,
                                                uint8_t snc_max_stall_cycles)
{
        ASSERT_ERROR(master_priorities->ram1_dma < 3);
        ASSERT_ERROR(master_priorities->ram2_dma < 3);
        ASSERT_ERROR(master_priorities->ram3_dma < 3);
        ASSERT_ERROR(master_priorities->ram4_dma < 3);
        ASSERT_ERROR(master_priorities->ram5_dma < 3);
        ASSERT_ERROR(master_priorities->ram6_dma < 3);
        ASSERT_ERROR(master_priorities->ram7_dma < 3);
        ASSERT_ERROR(master_priorities->ram8_dma < 3);

        ASSERT_ERROR((cpuc_max_stall_cycles > 0) && (cpuc_max_stall_cycles < 16));
        ASSERT_ERROR((cpus_max_stall_cycles > 0) && (cpus_max_stall_cycles < 16));
        ASSERT_ERROR((dma_max_stall_cycles > 0) && (dma_max_stall_cycles < 16));
        ASSERT_ERROR((snc_max_stall_cycles > 0) && (snc_max_stall_cycles < 16));

        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB1_AHB_CPUC_PRIO, master_priorities->ram1_cpuc);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB2_AHB_CPUC_PRIO, master_priorities->ram2_cpuc);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB3_AHB_CPUC_PRIO, master_priorities->ram3_cpuc);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB_CPUC_MAX_STALL, cpuc_max_stall_cycles);

        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB1_AHB_CPUS_PRIO, master_priorities->ram1_cpus);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB2_AHB_CPUS_PRIO, master_priorities->ram2_cpus);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB3_AHB_CPUS_PRIO, master_priorities->ram3_cpus);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB4_AHB_CPUS_PRIO, master_priorities->ram4_cpus);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB5_AHB_CPUS_PRIO, master_priorities->ram5_cpus);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB6_AHB_CPUS_PRIO, master_priorities->ram6_cpus);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB7_AHB_CPUS_PRIO, master_priorities->ram7_cpus);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB8_AHB_CPUS_PRIO, master_priorities->ram8_cpus);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB_CPUS_MAX_STALL, cpus_max_stall_cycles);

        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB1_AHB_DMA_PRIO, master_priorities->ram1_dma);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB2_AHB_DMA_PRIO, master_priorities->ram2_dma);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB3_AHB_DMA_PRIO, master_priorities->ram3_dma);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB4_AHB_DMA_PRIO, master_priorities->ram4_dma);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB5_AHB_DMA_PRIO, master_priorities->ram5_dma);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB6_AHB_DMA_PRIO, master_priorities->ram6_dma);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB7_AHB_DMA_PRIO, master_priorities->ram7_dma);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB8_AHB_DMA_PRIO, master_priorities->ram8_dma);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB_DMA_MAX_STALL, dma_max_stall_cycles);

        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB1_SNC_PRIO, master_priorities->ram1_snc);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB1_4_REG, ARB2_SNC_PRIO, master_priorities->ram2_snc);
        REG_SETF(MEMCTRL, MEM_PRIO_ARB5_8_REG, ARB8_SNC_PRIO, master_priorities->ram8_snc);
        REG_SETF(MEMCTRL, MEM_STALL_REG, SNC_MAX_STALL, snc_max_stall_cycles);
}

