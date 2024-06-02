/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_EMMC eMMC Driver
 * \{
 * \brief embedded Multi-Media Card (eMMC) Low Level Driver
 */

/**
 *****************************************************************************************
 *
 * @file hw_emmc.h
 *
 * @brief Definition of API for the embedded Multi-Media Card (eMMC) Low Level Driver.
 *
 * @warning eMMC is not supported in a **DA14705** or a **DA14706** device variant.
 *          A compilation error is produced in case the `dg_configUSE_HW_EMMC` is accidentally enabled.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_EMMC_H_
#define HW_EMMC_H_

#if (dg_configUSE_HW_EMMC == 1)

#include <stdbool.h>
#include "hw_sdhc.h"

/**
 * \brief Supported card command classes
 */
typedef enum {
        HW_EMMC_CARD_CMD_CLASS_0_BASIC = (1 << 0),      /**< Class 0: Basic */
        HW_EMMC_CARD_CMD_CLASS_1_STREAM_RD = (1 << 1),  /**< Class 1: Stream read */
        HW_EMMC_CARD_CMD_CLASS_2_BLK_RD = (1 << 2),     /**< Class 2: Block read */
        HW_EMMC_CARD_CMD_CLASS_3_STREAM_WR = (1 << 3),  /**< Class 3: Stream write */
        HW_EMMC_CARD_CMD_CLASS_4_BLK_WR = (1 << 4),     /**< Class 4: Block write */
        HW_EMMC_CARD_CMD_CLASS_5_ERASE = (1 << 5),      /**< Class 5: Erase */
        HW_EMMC_CARD_CMD_CLASS_6_WP = (1 << 6),         /**< Class 6: Write protection */
        HW_EMMC_CARD_CMD_CLASS_7_LOCK = (1 << 7),       /**< Class 7: Lock card */
        HW_EMMC_CARD_CMD_CLASS_8_APP = (1 << 8),        /**< Class 8: Application specific */
        HW_EMMC_CARD_CMD_CLASS_9_IO = (1 << 9),         /**< Class 9: I/O mode */

        /* JESD84-A441: reserved above Class 9 */
} HW_EMMC_CARD_CMD_CLASS;

/**
 * This macro defines the mask with active and implemented normal interrupts
 *
 * Note: these are mandatory interrupts
 */
#define HW_EMMC_ACTIVE_NORMAL_INTERRUPTS_MASK           REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, BUF_RD_READY_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, BUF_WR_READY_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, DMA_INTERRUPT_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, BGAP_EVENT_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, XFER_COMPLETE_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, CMD_COMPLETE_STAT_EN)

/**
 * This macro defines the mask with non-implemented/supported/applicable normal interrupts
 *
 */
#define HW_EMMC_NON_IMPL_NORMAL_INTERRUPTS_MASK         REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, CQE_EVENT_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, FX_EVENT_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, RE_TUNE_EVENT_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, INT_C_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, INT_B_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, INT_A_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_INTERRUPT_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_REMOVAL_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_INSERTION_STAT_EN)

/**
 * This macro defines the mask with active and implemented error interrupts
 */
#define HW_EMMC_ACTIVE_ERROR_INTERRUPTS_MASK            REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, RESP_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, ADMA_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, AUTO_CMD_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, CUR_LMT_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, DATA_END_BIT_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, DATA_CRC_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, DATA_TOUT_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_IDX_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_END_BIT_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_CRC_ERR_STAT_EN) | \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_TOUT_ERR_STAT_EN)

/**
 * This macro defines the mask with non-implemented/supported/applicable error interrupts
 */
#define HW_EMMC_NON_IMPL_ERROR_INTERRUPTS_MASK          REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, VENDOR_ERR_STAT_EN3) \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, VENDOR_ERR_STAT_EN2) \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, VENDOR_ERR_STAT_EN1) \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, BOOT_ACK_ERR_STAT_EN) \
                                                        REG_MSK(EMMC, EMMC_ERROR_INT_STAT_EN_R_REG, TUNING_ERR_STAT_EN)

#define HW_EMMC_HC_TIMEOUT_ERASE_FACTOR_MS              (300)   /**< Factor used to calculate the High Capacity (HC) erase/trim timeout, in ms */

/**
 * \brief eMMC card access data structure
 *
 */
typedef struct {
        uint32_t                bus_speed;              /**< Bus speed in Hz */
        uint32_t                s_a_timeout_usec;       /**< S_A_TIMEOUT defined in EXT_CSD[217], in usec */

        uint32_t                read_timeout_ms;        /**< Read block timeout in msec */
        uint32_t                write_timeout_ms;       /**< Write block timeout in msec */
} hw_emmc_card_access_t;

/**
 * \brief eMMC saved data/context structure
 *
 */
typedef struct {
        uint16_t                rca;                    /**< RCA set to card */
        hw_sdhc_emmc_cid_t      cid;                    /**< CID read from card */
        hw_sdhc_emmc_csd_t      csd;                    /**< CSD read from card */
        hw_sdhc_emmc_ext_csd_t  ext_csd;                /**< EXT_CSD read from card */
        hw_emmc_card_access_t   card_access_data;       /**< Card access data calculated using CSD and EXT_CSD registers */
} hw_emmc_context_data_t;

/**
 * \brief CSD programmable part structure, bits 8:15
 *
 */
typedef __PACKED_STRUCT {
        uint8_t                 ecc:2;                  /**< Defines the Error Correction Code (ECC) that was used for storing data on the card */
        uint8_t                 file_format:2;          /**< Indicates the file format on the card */
        uint8_t                 tmp_write_protect:1;    /**< Temporarily protects the whole card content from being overwritten or erased */
        uint8_t                 perm_write_protect:1;   /**< Permanently protects the whole card content from being overwritten or erased */
        uint8_t                 copy:1;                 /**< Defines if the contents is original (= 0) or has been copied (= 1) */
        uint8_t                 file_format_grp:1;      /**< Indicates the selected group of file formats */
} _hw_emmc_prg_csd_t;

typedef union {
        uint8_t                 prg_csd_val;            /**< value */
        _hw_emmc_prg_csd_t      prg_csd;                /**< fields */
} hw_emmc_prg_csd_t;

/*
 *****************************************************************************************
 *
 * eMMC API Functions.
 *
 *****************************************************************************************
 */
/**
 * \brief Enable Host Controller (HC)
 *
 * Check the status of External Memory Controller Power Domain
 * Set CLK_PDCTRL_REG: clock divider, invert RX/TX clock, enable the HC clock
 *
 * \param [in] id               SDHC controller instance
 * \param [in] config           Configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_enable(HW_SDHC_ID id, const hw_sdhc_pdctrl_reg_config_t *config);

/**
 * \brief Disable Host Controller (HC)
 *
 * Set CLK_PDCTRL_REG: disable the HC clock
 *
 * \note This function should be called after HC de-initialization
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_disable(const HW_SDHC_ID id);

/**
 * \brief Hardware reset the eMMC card
 *
 * \note This function should be called after enabling the HC, since it uses HC registers
 * \note By default HW reset is not enabled in an eMMC card and thus it is ignored by it
 *       EXT_CSD:RST_n_FUNCTION = 0x00
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rst_pulse_us     reset pulse width in us, min value = 1us
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_hw_reset_card(HW_SDHC_ID id, uint32_t rst_pulse_us);

/**
 * \brief eMMC initialization
 *
 * After initialization, Host Controller and Card should be ready to transfer data.
 *
 * \note This function should be called after HC enable
 *
 * \param [in] id                       SDHC controller instance
 * \param [in] config                   Configuration structure
 * \param [in] cb                       Callback function. If NULL then data transfers are blocking
 * \param [out] ptr_emmc_context        Pointer to the address of the emmc context data stored locally in the driver
 *                                      The returned pointer should be considered valid,
 *                                      only when HW_SDHC_STATUS_SUCCESS is returned and
 *                                      can be used for the lifetime of the driver
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_init(HW_SDHC_ID id, const hw_sdhc_config_t *config, hw_sdhc_event_callback_t cb, const hw_emmc_context_data_t **ptr_emmc_context);

/**
 * \brief eMMC de-initialization
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_deinit(HW_SDHC_ID id);

/**
 * \brief Check if controller is busy
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if controller is not busy
 *         HW_SDHC_STATUS_ERROR_OPERATION_IN_PROGRESS if controller is busy
 *         Otherwise, an error id
 */
HW_SDHC_STATUS hw_emmc_is_busy(HW_SDHC_ID id);

/**
 * \brief Set data bus width using SWITCH command (CMD6)
 *
 * Command SEND_STATUS (CMD13) is also called to check the value of card status bit 7 (SWITCH_ERROR)
 * If the operation does not fail, the emmc_context is updated.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_width        bus width value
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_set_data_bus_width(HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width);

/**
 * \brief Set data bus speed using SWITCH command (CMD6)
 *
 * Command SEND_STATUS (CMD13) is also called to check the value of card status bit 7 (SWITCH_ERROR)
 * If the operation does not fail, the emmc_context is updated.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] speed_mode       speed mode value
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_set_speed_mode(HW_SDHC_ID id, uint8_t speed_mode);

/**
 * \brief Set data bus width, speed mode, speed/frequency, drive strength
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_config       Data bus configuration
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_setup_data_bus(HW_SDHC_ID id, const hw_sdhc_bus_config_t *bus_config);

/**
 * \brief Issue CMD with Data Transfer (Non-DMA/PIO, SDMA, ADMA2)
 *
 * \note PIO: Programmed I/O
 * \note SDMA: Single operation DMA
 * \note ADMA2: Advanced DMA
 *
 * To check whether a data xfer command is sent (CMD8, 17, 18, 24, 25), the following
 * register fields should be read:
 *   EMMC_NORMAL_INT_STAT_R_REG.BUF_RD_READY/BUF_WR_READY = 1
 *   EMMC_NORMAL_INT_STAT_R_REG.CMD_COMPLETE = 1
 *   EMMC_PSTATE_REG.BUF_RD_ENABLE/BUF_WR_ENABLE = 1
 *   EMMC_PSTATE_REG.RD_XFER_ACTIVE/WR_XFER_ACTIVE = 1
 *
 * If CMD_COMPLETE is reset then BUF_RD_READY/BUF_WR_READY are also reset.
 * This should be avoided in order to start the following data xfer successfully.
 *
 * \param [in] id               SDHC controller instance
 * \param [inout] config        Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Parameter config should be valid until the transaction is complete,
 *       since it contains the data read or written.
 */
HW_SDHC_STATUS hw_emmc_data_xfer(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Data transfer abort
 *
 * \note If called when no data transfer is active then return HW_SDHC_STATUS_SUCCESS
 *
 * \param [in] id               SDHC controller instance
 * \param [in] abort_method     Abort method: synchronous or asynchronous
 * \param [in] tout_ms          timeout for the active data transfer, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_abort_xfer(HW_SDHC_ID id, HW_SDHC_ABORT_METHOD abort_method, uint32_t tout_ms);

/**
 * \brief Data transfer error recovery
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Instead of HW_SDHC_STATUS_RECOVERABLE_ERROR, return HW_SDHC_STATUS_SUCCESS
 *       to be consistent with other API functions return values
 */
HW_SDHC_STATUS hw_emmc_error_recovery(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Read Card Identification Register (CID) using the command SEND_CID (CMD10)
 *        and update the driver emmc_context, accordingly.
 *        The pointer to emmc_context is returned at hw_emmc_init().
 *
 * CID register is 16 bytes long, including the CRC7 field
 * Although the card sends CRC7, the host controller does not include it in the command response
 * Thus, the command response is 15 bytes long
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note The function hw_emmc_program_cid() programs a new value of the CID register,
 *       and updates the driver emmc_context, accordingly.
 *
 */
HW_SDHC_STATUS hw_emmc_get_card_cid(HW_SDHC_ID id);

/**
 * \brief Program the Card Identification Register (CID), using the command PROGRAM_CID (CMD26)
 *
 * Prepare the buffer to program the CID (16 bytes):
 * - Reverse the order of the CID bytes so that the MSB is first
 * - Calculate the new CRC7
 *
 * After programming the CID, the command SEND_STATUS (CMD13) is sent to check
 * whether the card status bit CID/CSD_OVERWRITE is set
 *
 * Local CID data is updated if the returned value is HW_SDHC_STATUS_SUCCESS.
 * If hw_emmc_get_card_cid() was previously called, then the returned address (cid)
 * points to the updated data as well.
 *
 * \note Normally, the CID register has already been written by the manufacturer and
 *       can not be overwritten
 *
 * \param [in] id               SDHC controller instance
 * \param [in] prg_cid          CID value to be programmed, byte order is reversed and CRC7 is added
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_program_cid(HW_SDHC_ID id, const hw_sdhc_emmc_cid_t *prg_cid);

/**
 * \brief Read Card Specific Data Register (CSD) using the command SEND_CSD (CMD9)
 *        and update the driver emmc_context, accordingly.
 *        The pointer to emmc_context is returned at hw_emmc_init().
 *
 * CSD register is 16 bytes long, including the CRC7 field
 * Although the card sends CRC7, the host controller does not include it in the command response
 * Thus, the command response is 15 bytes long
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note The function hw_emmc_program_csd() programs a new value of the CSD register,
 *       and updates the driver emmc_context, accordingly.
 */
HW_SDHC_STATUS hw_emmc_get_card_csd(HW_SDHC_ID id);

/**
 * \brief Program the programmable part of Card Specific Data Register (CSD), using the command PROGRAM_CSD (CMD27)
 *        The programmable bits of CSD are 8 to 15
 *
 * Prepare the buffer to program the CSD (16 bytes):
 * - The read-only part of the CSD should match the card content
 * - Reverse the order of the CSD bytes stored in the eMMC context so that the MSB is first
 * - Set the programmable part
 * - Calculate the new CRC7
 *
 * After programming the CSD, check if card status CID/CSD_OVERWRITE bit is set
 *
 * Local CSD data is updated if the returned value is HW_SDHC_STATUS_SUCCESS.
 * If hw_emmc_get_card_csd() was previously called, then the returned address (csd)
 * points to the updated data as well.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] prg_csd          programmable part of CSD to be programmed
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_program_csd(HW_SDHC_ID id, const hw_emmc_prg_csd_t prg_csd);

/**
 * \brief Read Extended Card Specific Data Register (EXT_CSD) using the command SEND_EXT_CSD (CMD8)
 *        and update the driver emmc_context, accordingly.
 *        The pointer to emmc_context is returned at hw_emmc_init().
 *
 * EXT_CSD register is 512 bytes long
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_get_card_ext_csd(HW_SDHC_ID id);

/**
 * \brief Get card status register using the command SEND_STATUS (CMD13)
 *
 * \param [in] id               SDHC controller instance
 * \param [out] status_reg      Pointer to card status register
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_get_card_status_register(HW_SDHC_ID id, uint32_t *status_reg);

/**
 * \brief Switch an eMMC card to Sleep state
 *
 * \note If the card is not in Standby state, try to switch to Standby state and then execute the sleep command
 *       If the card cannot switch to Standby state (at previous step) then return an error
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout for state transition, in ms
 *                              If the value is 0, the maximum timeout value is used as defined in EXT_CSD[217] = S_A_TIMEOUT
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_sleep(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Switch an eMMC card from Sleep to Transfer state
 *
 * \note If the card is in Sleep state, it reacts only to the commands RESET (CMD0) and AWAKE (CMD5)
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout for state transition, in ms
 *                              If the value is 0, the maximum timeout value is used as defined in EXT_CSD[217] = S_A_TIMEOUT
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_awake(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Start Host Controller internal and SD Bus clocks
 *
 * \note Can be used with awake command after a sleep
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_start_hc_clocks(HW_SDHC_ID id);

/**
 * \brief Stop Host Controller internal and SD Bus clocks
 *
 * \note Can be used with sleep command for low power consumption
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_stop_hc_clocks(HW_SDHC_ID id);

/**
 * \brief Erase the specified erase groups using CMD35, 36 and 38
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * \param [in] id                       SDHC controller instance
 * \param [in] start_erase_group        start erase group, valid values = 0..(SEC_COUNT/erase_group_size)-1
 * \param [in] end_erase_group          end erase group, valid values = 0..(SEC_COUNT/erase_group_size)-1
 *                                      The start group cannot be greater than the end group
 *                                      SEC_COUNT = EXT_CSD[215:212] = max sector count of the device
 * \param [in] tout_ms                  timeout in msec, should be the multiple of the number of the erase groups involved
 *                                      If the value is 0, the maximum timeout value is used as defined in EXT_CSD
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_erase_groups(HW_SDHC_ID id, uint32_t start_erase_group, uint32_t end_erase_group, uint32_t tout_ms);

/**
 * \brief Secure Erase the specified erase groups using CMD35, 36 and 38
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * Moreover, the BIT0 of EXT_CSD[231] = SEC_FEATURE_SUPPORT should be already set
 *
 * \param [in] id                       SDHC controller instance
 * \param [in] start_erase_group        start erase group, valid values = 0..(SEC_COUNT/erase_group_size)-1
 * \param [in] end_erase_group          end erase group, valid values = 0..(SEC_COUNT/erase_group_size)-1
 *                                      The start group cannot be greater than the end group
 *                                      SEC_COUNT = EXT_CSD[215:212] = max sector count of the device
 * \param [in] tout_ms                  timeout in msec, should be the multiple of the number of the erase groups involved
 *                                      If the value is 0, the maximum timeout value is used as defined in EXT_CSD
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_erase_groups_secure(HW_SDHC_ID id, uint32_t start_erase_group, uint32_t end_erase_group, uint32_t tout_ms);

/**
 * \brief Trim (erase) the specified card sectors/blocks using CMD35, 36 and 38
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * Moreover, the BIT4 of EXT_CSD[231] = SEC_FEATURE_SUPPORT should be already set
 *
 * \param [in] id               SDHC controller instance
 * \param [in] start_addr       start address in sectors/blocks, valid values = 0..SEC_COUNT-1
 * \param [in] end_addr         end address in sectors/blocks, valid values = 0..SEC_COUNT-1
 *                              start_addr cannot be greater than the end_addr
 *                              SEC_COUNT = EXT_CSD[215:212] = max sector count of the device
 * \param [in] tout_ms          timeout in msec, should be the multiple of the number of the erase groups involved
 *                              If the value is 0, the maximum timeout value is used as defined in EXT_CSD
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_trim_blocks(HW_SDHC_ID id, uint32_t start_addr, uint32_t end_addr, uint32_t tout_ms);

/**
 * \brief Mark the specified card sectors/blocks for Secure Trim (erase) using CMD35, 36 and 38
 * This is the Secure Trim Step 1
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * Moreover, the BIT0 and BIT4 of EXT_CSD[231] = SEC_FEATURE_SUPPORT should be already set
 *
 * \param [in] id               SDHC controller instance
 * \param [in] start_addr       start address in sectors/blocks, valid values = 0..SEC_COUNT-1
 * \param [in] end_addr         end address in sectors/blocks, valid values = 0..SEC_COUNT-1
 *                              start_addr cannot be greater than the end_addr
 *                              SEC_COUNT = EXT_CSD[215:212] = max sector count of the device
 * \param [in] tout_ms          timeout in msec, should be the multiple of the number of the erase groups involved
 *                              If the value is 0, the maximum timeout value is used as defined in EXT_CSD
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_trim_mark_blocks_secure(HW_SDHC_ID id, uint32_t start_addr, uint32_t end_addr, uint32_t tout_ms);

/**
 * \brief Secure Trim (erase) the specified card sectors/blocks using CMD35, 36 and 38
 * This is the Secure Trim Step 2
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * Moreover, the BIT0 and BIT4 of EXT_CSD[231] = SEC_FEATURE_SUPPORT should be already set
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout in msec, should be the multiple of the number of the erase groups involved
 *                              The value cannot be 0
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_emmc_trim_blocks_secure(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Set a password that can be used to lock the card
 *
 * The password length of an eMMC card is 1 to 16 bytes.
 * The user can set the password and lock the card using a single command.
 * The card status bits HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED and HW_SDHC_CARD_STATUS_CARD_IS_LOCKED
 * are checked before returning.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] pwd              pointer to the password bytes
 * \param [in] len              password length
 * \param [in] lock             select to lock the card or not
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note
 * An attempt to use password protection features (CMD42) on a card having password permanently disabled
 * will fail and the LOCK_UNLOCK_FAILED (bit 24) error bit will be set in the status register.
 * The password protection feature can be disabled permanently by setting the permanent password disable
 * bit (PERM_PSWD_DIS bit in the EXT_CSD byte [171]).
 */
HW_SDHC_STATUS hw_emmc_card_set_password(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len, bool lock);

/**
 * \brief Clear the password that has been set to lock the card
 *
 * The password length of an eMMC card is 1 to 16 bytes.
 * The user should use the correct password for this operation.
 * The card status bit HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED is checked before returning.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] pwd              pointer to the password bytes
 * \param [in] len              password length
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note
 * An attempt to use password protection features (CMD42) on a card having password permanently disabled
 * will fail and the LOCK_UNLOCK_FAILED (bit 24) error bit will be set in the status register.
 * The password protection feature can be disabled permanently by setting the permanent password disable
 * bit (PERM_PSWD_DIS bit in the EXT_CSD byte [171]).
 */
HW_SDHC_STATUS hw_emmc_card_clr_password(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len);

/**
 * \brief Replace the password that has been set to lock the card with a new one
 *
 * The password length of an eMMC card is 1 to 16 bytes.
 * The user can replace the password and lock the card using a single command.
 * The card status bits HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED and HW_SDHC_CARD_STATUS_CARD_IS_LOCKED
 * are checked before returning.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] old_pwd          pointer to the old/current password bytes
 * \param [in] old_len          old/current password length
 * \param [in] new_pwd          pointer to the new password bytes
 * \param [in] new_len          new password length
 * \param [in] lock             select to lock the card or not
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note
 * An attempt to use password protection features (CMD42) on a card having password permanently disabled
 * will fail and the LOCK_UNLOCK_FAILED (bit 24) error bit will be set in the status register.
 * The password protection feature can be disabled permanently by setting the permanent password disable
 * bit (PERM_PSWD_DIS bit in the EXT_CSD byte [171]).
 */
HW_SDHC_STATUS hw_emmc_card_replace_password(HW_SDHC_ID id, const uint8_t *old_pwd, uint8_t old_len, const uint8_t *new_pwd, uint8_t new_len, bool lock);

/**
 * \brief Lock the card using the password that has been already set
 *
 * The password length of an eMMC card is 1 to 16 bytes.
 * The user should use the correct password to lock the card.
 * The card status bits HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED and HW_SDHC_CARD_STATUS_CARD_IS_LOCKED
 * are checked before returning.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] pwd              pointer to the password bytes
 * \param [in] len              password length
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note
 * An attempt to use password protection features (CMD42) on a card having password permanently disabled
 * will fail and the LOCK_UNLOCK_FAILED (bit 24) error bit will be set in the status register.
 * The password protection feature can be disabled permanently by setting the permanent password disable
 * bit (PERM_PSWD_DIS bit in the EXT_CSD byte [171]).
 */
HW_SDHC_STATUS hw_emmc_card_lock(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len);

/**
 * \brief Unlock the card using the password that has been already set
 *
 * The password length of an eMMC card is 1 to 16 bytes.
 * The user should use the correct password to unlock the card.
 * The card status bits HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED and HW_SDHC_CARD_STATUS_CARD_IS_LOCKED
 * are checked before returning.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] pwd              pointer to the password bytes
 * \param [in] len              password length
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note
 * An attempt to use password protection features (CMD42) on a card having password permanently disabled
 * will fail and the LOCK_UNLOCK_FAILED (bit 24) error bit will be set in the status register.
 * The password protection feature can be disabled permanently by setting the permanent password disable
 * bit (PERM_PSWD_DIS bit in the EXT_CSD byte [171]).
 */
HW_SDHC_STATUS hw_emmc_card_unlock(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len);

/**
 * \brief Erase all the card data content along with the password content (Forced Erase)
 *
 * An attempt to force erase an unlocked card will fail and HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED
 * bit will be set in the status register.
 * This operation can be used in case that the user forgot the password and the card is locked.
 * The card status bit HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED is checked before returning.
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note
 * An attempt to use password protection features (CMD42) on a card having password permanently disabled
 * will fail and the LOCK_UNLOCK_FAILED (bit 24) error bit will be set in the status register.
 * The password protection feature can be disabled permanently by setting the permanent password disable
 * bit (PERM_PSWD_DIS bit in the EXT_CSD byte [171]).
 */
HW_SDHC_STATUS hw_emmc_card_force_erase(HW_SDHC_ID id);

/**
 * \brief Get the erase group size, based on ERASE_GROUP_DEF = EXT_CSD[175]
 *
 * If ERASE_GROUP_DEF == 1 then HC_ERASE_GRP_SIZE = EXT_CSD[224] is used (if non-zero),
 * which defines the erase-unit size for high-capacity memory, else the default
 * unit is used.
 *
 * This function should be called after CSD and EXT_CSD are read/updated.
 * More specifically, after hw_emmc_init() or any EXT_CSD modification using CMD6 (SWITCH).
 *
 * \return size in sectors, non-zero value
 */
uint32_t hw_emmc_get_erase_group_size(void);

/**
 * \brief Get the write protect group size, based on ERASE_GROUP_DEF = EXT_CSD[175]
 *
 * If ERASE_GROUP_DEF == 1 then HC_WP_GRP_SIZE = EXT_CSD[221] is used (if non-zero),
 * which defines the write protect group size for high-capacity memory, else the default
 * size is used.
 *
 * This function should be called after CSD and EXT_CSD are read/updated.
 * More specifically, after hw_emmc_init() or any EXT_CSD modification using CMD6 (SWITCH).
 *
 * \return size in sectors, non-zero value
 */
uint32_t hw_emmc_get_wp_group_size(void);

/**
 * \brief Get the erase timeout in msec of one logical erase group, based on ERASE_GROUP_DEF = EXT_CSD[175]
 *
 * If ERASE_GROUP_DEF == 1 then ERASE_TIMEOUT_MULT = EXT_CSD[223] is used (if non-zero)
 * to calculate the erase timeout for high-capacity memory, else the default
 * value is used.
 *
 * This function should be called after CSD and EXT_CSD are read/updated.
 * More specifically, after hw_emmc_init() or any EXT_CSD modification using CMD6 (SWITCH).
 *
 * \return non-zero value
 */
uint32_t hw_emmc_get_erase_timeout_ms(void);

/**
 * \brief Get the secure erase timeout in msec of one logical erase group, based on ERASE_GROUP_DEF = EXT_CSD[175]
 *
 * If ERASE_GROUP_DEF == 1 then SEC_ERASE_MULT = EXT_CSD[230] is used (if defined)
 * to calculate the secure erase timeout for high-capacity memory, else the default
 * value is used.
 *
 * This function should be called after CSD and EXT_CSD are read/updated.
 * More specifically, after hw_emmc_init() or any EXT_CSD modification using CMD6 (SWITCH).
 *
 * \return non-zero value
 */
uint32_t hw_emmc_get_sec_erase_timeout_ms(void);

/**
 * \brief Get the trim timeout in msec of one logical erase group
 *
 * Use TRIM_MULT = EXT_CSD[232] (if defined) to calculate the trim timeout.
 * It is the same value for both default and high-capacity memories.
 *
 * This function should be called after CSD and EXT_CSD are read.
 * More specifically, after hw_emmc_init().
 *
 * \return non-zero value
 */
uint32_t hw_emmc_get_trim_timeout_ms(void);

/**
 * \brief Get the secure trim timeout in msec of one logical erase group
 *
 * Use SEC_TRIM_MULT = EXT_CSD[229] and ERASE_TIMEOUT_MULT = EXT_CSD[223] (if defined) to calculate the secure trim timeout.
 * It is the same value for both default and high-capacity memories.
 *
 * This function should be called after CSD and EXT_CSD are read.
 * More specifically, after hw_emmc_init().
 *
 * \return non-zero value
 */
uint32_t hw_emmc_get_sec_trim_timeout_ms(void);

#endif /* (dg_configUSE_HW_EMMC == 1) */

#endif /* HW_EMMC_H_ */

/**
 * \}
 * \}
 */
