/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_MEMCTRL Memory Controller
 * \{
 * \brief Memory Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_memctrl.h
 *
 * @brief Definition of API for the Memory Controller Low Level Driver.
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */
#ifndef HW_MEMCTRL_H_
#define HW_MEMCTRL_H_

#include <sdk_defs.h>

/**
 * \brief Priorities for CPUC/CPUS/DMA/SNC accesses to RAM cells 1-8
 *
 */
typedef struct {
        uint8_t ram1_cpuc : 2;  /**< Priority for CPUC access to RAM cell 1;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram1_cpus : 2;  /**< Priority for CPUS access to RAM cell 1;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram1_dma  : 2;  /**< Priority for DMA access to RAM cell 1;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram1_snc  : 2;  /**< Priority for SNC access to RAM cell 1;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram2_cpuc : 2;  /**< Priority for CPUC access to RAM cell 2;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram2_cpus : 2;  /**< Priority for CPUS access to RAM cell 2;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram2_dma  : 2;  /**< Priority for DMA access to RAM cell 2;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram2_snc  : 2;  /**< Priority for SNC access to RAM cell 2;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram3_cpuc : 2;  /**< Priority for CPUC access to RAM cell 3;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram3_cpus : 2;  /**< Priority for CPUS access to RAM cell 3;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram3_dma  : 2;  /**< Priority for DMA access to RAM cell 3;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram4_cpus : 2;  /**< Priority for CPUS access to RAM cell 4;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram4_dma  : 2;  /**< Priority for DMA access to RAM cell 4;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram5_cpus : 2;  /**< Priority for CPUS access to RAM cell 5;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram5_dma  : 2;  /**< Priority for DMA access to RAM cell 5;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram6_cpus : 2;  /**< Priority for CPUS access to RAM cell 6;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram6_dma  : 2;  /**< Priority for DMA access to RAM cell 6;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram7_cpus : 2;  /**< Priority for CPUS access to RAM cell 7;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram7_dma  : 2;  /**< Priority for DMA access to RAM cell 7;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram8_cpus : 2;  /**< Priority for CPUS access to RAM cell 8;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint8_t ram8_dma  : 2;  /**< Priority for DMA access to RAM cell 8;
                                     possible values: 0 (= low/default) - 2 (= high) */
        uint8_t ram8_snc  : 2;  /**< Priority for SNC access to RAM cell 8;
                                     possible values: 0 (= low/default) - 3 (= top) */
        uint32_t padding  : 20; /* Explicit padding */
} memctrl_master_priorities_t;

/**
 * \brief Resets memory controller's configuration.
 *
 * Shall be used only when the CMAC master is disabled.
 * In the case of DA1469X, the SNC master shall be disabled as well.
 */
void hw_memctrl_reset(void);

/**
 * \brief Configures CMAC code, data and shared regions.
 *
 * \param [in] data_base_addr   CMAC data address. CMAC 0x20000000 address is remapped to this value.
 *                              Must be a multiple of 4.
 * \param [in] shared_base_addr CMAC code address. Must be a multiple of 1024.
 * \param [in] end_addr         The upper bound of RAM region that CMAC can access.
 *                              Must end at 1024-byte boundary (10 last bits 0x3FF).
 */
void hw_memctrl_config_cmac_region(uint32_t data_base_addr, uint32_t shared_base_addr,
                                        uint32_t end_addr);


/**
 * \brief Configures RAM access priority for CPUC, CPUS, DMA and SNC.
 *
 * CMAC has always priority over CPUC, CPUS, DMA and SNC.
 *
 * When CPUC/CPUS/DMA/SNC request access to the same RAM cell, the priority fields determine
 * which master will gain access first. For the masters that did not get priority there is an
 * internal counter (its initial value is equal to the respective number of stall cycles)
 * that decreases by one. When the counter reaches zero, the specific master will gain access
 * regardless of its priority for a single cycle and the internal counter will be reset again to the
 * initial number of stall cycles. This is done to avoid starvation of low-priority masters.
 *
 * A possible mapping of priorities to priority/stall cycle pair values could be the following:
 * - HIGHEST: PRIO 2, STALL 3
 * - HIGH: PRIO 2, STALL 6
 * - MEDIUM: PRIO 1, STALL 9
 * - LOW: PRIO 0, STALL 12
 * - LOWEST: PRIO 0, STALL 15
 *
 * Configuring two masters with the same number of stall cycles shall be avoided, since the field
 * was added to differentiate between masters.
 *
 * \param [in] master_priorities CPUC/CPUS/DMA/SNC priorities
 * \param [in] cpuc_max_stall_cycles Maximum allowed number of stall cycles for CPUC (1 - 15)
 * \param [in] cpus_max_stall_cycles Maximum allowed number of stall cycles for CPUS (1 - 15)
 * \param [in] dma_max_stall_cycles Maximum allowed number of stall cycles for DMA (1 - 15)
 * \param [in] snc_max_stall_cycles Maximum allowed number of stall cycles for SNC (1 - 15)
 */
void hw_memctrl_config_master_priorities(memctrl_master_priorities_t *master_priorities,
                                                uint8_t cpuc_max_stall_cycles,
                                                uint8_t cpus_max_stall_cycles,
                                                uint8_t dma_max_stall_cycles,
                                                uint8_t snc_max_stall_cycles);

#endif /* HW_MEMCTRL_H_ */

/**
\}
\}
*/
