/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_SDHC SD Host Controller
 * \{
 * \brief SD Host Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_sdhc.h
 *
 * @brief Definition of API for the SD Host Controller Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_SDHC_H_
#define HW_SDHC_H_


#define __HW_SDHC_USE_HW_EMMC_ONLY      (1)

#if (dg_configUSE_HW_EMMC == 1) || (__HW_SDHC_USE_HW_EMMC_ONLY == 0)

#include "sdk_defs.h"


/**
 * \brief SDHC Controller base address
 *
 */
typedef EMMC_Type*                      HW_SDHC_ID;
#define HW_EMMCC                        ((HW_SDHC_ID) EMMC_BASE)
#define HW_SDHC_INT(id)                 ((id) == HW_EMMCC ? (eMMC_IRQn) : *(uint32_t *)(UINT32_MAX))
#define HW_SDHC_IDX(id)                 ((id) == HW_EMMCC ? 0 : *(uint32_t *)(UINT32_MAX))

#define HW_SDHC_DATA(id)                (context_p[HW_SDHC_IDX(id)])

/**
 * \brief Write a value to an SDHC register field
 *
 * \param [in] id       SDHC controller instance
 * \param [in] reg      the SDHC register
 * \param [in] field    the SDHC register field
 * \param [in] val      value to be written
 *
 * \sa HW_SDHC_REG_GETF
 */
#define HW_SDHC_REG_SETF(id, reg, field, val) \
        (id)->reg = (((id)->reg & ~(EMMC_##reg##_##field##_Msk)) | \
        ((EMMC_##reg##_##field##_Msk) & ((val) << (EMMC_##reg##_##field##_Pos))))

/**
 * \brief Get the value of an SDHC register field
 *
 * \param [in] id       SDHC controller instance
 * \param [in] reg      the SDHC register
 * \param [in] field    the SDHC register field
 *
 * \sa HW_SDHC_REG_SETF
 */
#define HW_SDHC_REG_GETF(id, reg, field) \
        (((id)->reg & (EMMC_##reg##_##field##_Msk)) >> (EMMC_##reg##_##field##_Pos))

/*
 *****************************************************************************************
 *
 * Public Macros.
 *
 *****************************************************************************************
 */
#define HW_SDHC_HC_CLOCK_GENERATOR_SUPPORTED            (0UL)     /**< Host Controller does not support a programmable clock generator */
#define HW_SDHC_SUPPORT_DDR                             (0UL)     /**< Host Controller does not support DDR */

#define HW_SDHC_DEFAULT_BLOCK_SIZE                      (512UL)   /**< Default block size in bytes */

#define HW_SDHC_MAX_UPPER_FREQ_SEL                      (3UL)     /**< CLK_CTRL_R: UPPER_FEQ_SEL max value */

#define HW_SDHC_MAX_XFER_BLOCK_SIZE                     (0x800UL) /**< BLOCKSIZE_R: transfer block size max non-zero value in bytes */

#define HW_SDHC_TOUT_CNT_MAX_REG_FIELD_VAL              (0x0EUL)  /**< TOUT_CNT: max value */

#define HW_SDHC_EMMC_BUS_SPEED_LEGACY_MAX               (26000000UL)    /**< eMMC max bus speed at Legacy mode, in Hz */
#define HW_SDHC_EMMC_BUS_SPEED_HS_SDR_MAX               (52000000UL)    /**< eMMC max bus speed at High Speed SDR mode, in Hz */
#if (HW_SDHC_SUPPORT_DDR == 1)
#define HW_SDHC_EMMC_BUS_SPEED_HS_DDR_MAX               (52000000UL)    /**< eMMC max bus speed at High Speed DDR mode, in Hz */
#endif

#define HW_SDHC_UHS_BUS_SPEED_SDR12_MAX                 (25000000UL)    /**< UHS max bus speed at SDR12 mode, in Hz */
#define HW_SDHC_UHS_BUS_SPEED_SDR25_MAX                 (50000000UL)    /**< UHS max bus speed at SDR25 mode, in Hz */
#if (HW_SDHC_SUPPORT_DDR == 1)
#define HW_SDHC_UHS_BUS_SPEED_DDR50_MAX                 (40000000UL)    /**< UHS max bus speed at DDR50 mode, in Hz */
#endif

#define HW_SDHC_CMD42_PWD_LEN_MAX                       (16UL)            /**< CMD42 valid password length is 1 to 16 Bytes */
#define HW_SDHC_CMD42_LEN_MAX                           (2UL + 2UL * HW_SDHC_CMD42_PWD_LEN_MAX)   /**< CMD42 maximum length in Bytes, in case of password replacement */

/*
 *****************************************************************************************
 *
 * Enum's.
 *
 *****************************************************************************************
 */

/**
 * \brief SDHC return status codes
 *
 */
typedef enum {
        HW_SDHC_STATUS_SUCCESS = (0UL),                         /**< Success */
        HW_SDHC_STATUS_ERROR,                                   /**< General error code */
        HW_SDHC_STATUS_ERROR_INVALID_PARAMETER,                 /**< A function argument is not valid */
        HW_SDHC_STATUS_ERROR_STATE_NOT_FREE,                    /**< Host Controller state is not free, i.e. it is occupied */
        HW_SDHC_STATUS_ERROR_OPERATION_IN_PROGRESS,             /**< Operation is still in progress */

        HW_SDHC_STATUS_HC_INVALID_VERSION,                      /**< Host Controller version is invalid/unexpected */
        HW_SDHC_STATUS_RECOVERABLE_ERROR,                       /**< Error recovery sequence returns a recoverable error status */
        HW_SDHC_STATUS_NON_RECOVERABLE_ERROR,                   /**< Error recovery sequence returns a non-recoverable error status */
        HW_SDHC_STATUS_ERROR_UNUSABLE_CARD,                     /**< The card is unusable */
        HW_SDHC_STATUS_ERROR_SUPPLIED_VOLTAGE,                  /**< Supplied voltage does not match with voltage window of card */
        HW_SDHC_STATUS_ERROR_CARD_IS_COMBO,                     /**< Card is combo (memory and I/O) */
        HW_SDHC_STATUS_ERROR_1V8,                               /**< Card does not support 1V8 bus signaling level */
        HW_SDHC_STATUS_ERROR_VOLTAGE_SWITCH,                    /**< Voltage switch error */
        HW_SDHC_STATUS_ERROR_CARD_REG_VAL_NOT_RECOGNIZED,       /**< Card register value is not recognized */

        HW_SDHC_STATUS_ERROR_RESPONSE_5,                        /**< Command response (R5) error */
        HW_SDHC_STATUS_ERROR_RESPONSE_6,                        /**< Command response (R6) error */

        HW_SDHC_STATUS_ERROR_TIMEOUT,                           /**< A Time Out error occurred */
        HW_SDHC_STATUS_ERROR_TIMEOUT_STOP_SD_CLK,               /**< SD Clock stop timeout */
        HW_SDHC_STATUS_ERROR_TIMEOUT_CMD_LINE,                  /**< CMD line de-activation timeout */
        HW_SDHC_STATUS_ERROR_TIMEOUT_DATA_LINE,                 /**< DAT line de-activation timeout */

        HW_SDHC_STATUS_ERROR_CMD_TOUT,                          /**< Command timeout error */
        HW_SDHC_STATUS_ERROR_CMD_CRC,                           /**< Command CRC error */
        HW_SDHC_STATUS_ERROR_CMD_END_BIT,                       /**< Command end bit error */
        HW_SDHC_STATUS_ERROR_CMD_IDX,                           /**< Command index error */
        HW_SDHC_STATUS_ERROR_DATA_TOUT,                         /**< Data timeout error */
        HW_SDHC_STATUS_ERROR_DATA_CRC,                          /**< Data CRC error */
        HW_SDHC_STATUS_ERROR_DATA_END_BIT,                      /**< Data end bit error */
        HW_SDHC_STATUS_ERROR_CUR_LMT,                           /**< Current limit error */
        HW_SDHC_STATUS_ERROR_AUTO_CMD,                          /**< Auto command error */
        HW_SDHC_STATUS_ERROR_ADMA_ERR,                          /**< Error during ADMA-based data transfer */
        HW_SDHC_STATUS_ERROR_RESP_ERR,                          /**< Host Controller response error check */
        HW_SDHC_STATUS_ERROR_INT_STAT_R,                        /**< Not-supported Error in Interrupt Status Register */

        HW_SDHC_STATUS_ERROR_PAGE_BOUNDARY,                     /**< SDMA Page Boundary error (DMA_INTERRUPT) */

        HW_SDHC_STATUS_ERROR_CARD_STATUS_SWITCH,                /**< Card status error: If set, the card did not switch to
                                                                     the expected mode as requested by the SWITCH command */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_ERASE_RESET,           /**< Card status error: An erase sequence was cleared before executing
                                                                     because an out of erase sequence command was received */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_WP_ERASE_SKIP,         /**< Card status error: Only partial address space was erased due to
                                                                     existing write protected blocks */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_CID_CSD_OVRWR,         /**< Card status error: CID/CSD program error */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_GEN_ERROR,             /**< Card status error: A generic card error related to the (and
                                                                     detected during) execution of the last host command
                                                                     (Undefined by the standard) */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_CC,                    /**< Card status error: A card error occurred, which is not related to
                                                                     the host command (Undefined by the standard) */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_ECC,                   /**< Card status error: Card internal ECC was applied but failed to
                                                                     correct the data */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_ILLEGAL_CMD,           /**< Card status error: Command not legal for the card state */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_COM_CRC,               /**< Card status error: The CRC check of the previous command failed */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_LOCK_UNLOCK_FAIL,      /**< Card status error: Set when a sequence or password error has
                                                                     been detected in lock/unlock card command */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED,        /**< Card status error: When set, signals that the card is locked by the host */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_WP_VIOLATION,          /**< Card status error: Attempt to program a write protected block */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_ERASE_PARAM,           /**< Card status error: An invalid selection of erase groups for erase occurred */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_ERASE_SEQ,             /**< Card status error: An error in the sequence of erase commands occurred */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_BLOCK_LEN,             /**< Card status error: Block length error */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_ADDRESS_MISALIGN,      /**< Card status error: Address misalign error */
        HW_SDHC_STATUS_ERROR_CARD_STATUS_ADDR_OUT_OF_RANGE,     /**< Card status error: Address out of range error */

        HW_SDHC_STATUS_ERROR_CARD_STATUS_ERRORS,                /**< Errors are found in card status */
} HW_SDHC_STATUS;

/**
 * \brief Events that trigger the event handler
 * These events come from the NORMAL_INT_STAT_R_REG and software defined events
 *
 */
typedef enum {
        HW_SDHC_EVENT_NONE                      = (0UL),                                /**< No event */
        // Events from NORMAL_INT_STAT_R_REG
        HW_SDHC_EVENT_CMD_COMPLETE              =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, CMD_COMPLETE)),       /**< Command Complete */
        HW_SDHC_EVENT_XFER_COMPLETE             =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, XFER_COMPLETE)),      /**< Host read/write transfer is complete */
        HW_SDHC_EVENT_BGAP_EVENT                =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, BGAP_EVENT)),         /**< This bit is set when both read/write transaction is stopped at the block gap */
        HW_SDHC_EVENT_DMA_INTERRUPT             =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, DMA_INTERRUPT)),      /**< Host controller detects an SDMA Buffer Boundary during transfer */
        HW_SDHC_EVENT_BUF_WR_READY              =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, BUF_WR_READY)),       /**< This bit is set if the Buffer Write Enable changes from 0 to 1 */
        HW_SDHC_EVENT_BUF_RD_READY              =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, BUF_RD_READY)),       /**< This bit is set if the Buffer Read Enable changes from 0 to 1 */
        HW_SDHC_EVENT_CARD_INSERTION            =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, CARD_INSERTION)),     /**< This bit is set if the Card Inserted in the Present State register changes from 0 to 1 */
        HW_SDHC_EVENT_CARD_REMOVAL              =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, CARD_REMOVAL_STAT_R)),/**< This bit is set if the Card Inserted in the Present State register changes from 1 to 0 */
        HW_SDHC_EVENT_CARD_INTERRUPT            =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, CARD_INTERRUPT)),     /**< The synchronized value of the DAT[1] interrupt input for SD mode */
        HW_SDHC_EVENT_INT_A                     =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, INT_A)),              /**< This bit is set if INT_A is enabled and if INT_A# pin is in low level. The INT_A# pin is not supported */
        HW_SDHC_EVENT_INT_B                     =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, INT_B)),              /**< This bit is set if INT_B is enabled and if INT_B# pin is in low level. The INT_B# pin is not supported */
        HW_SDHC_EVENT_INT_C                     =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, INT_C)),              /**< This bit is set if INT_C is enabled and if INT_C# pin is in low level. The INT_C# pin is not supported */
        HW_SDHC_EVENT_RE_TUNE_EVENT             =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, RE_TUNE_EVENT)),      /**< This bit is set if the Re-Tuning Request changes from 0 to 1 */
        HW_SDHC_EVENT_FX_EVENT                  =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, FX_EVENT)),           /**< This status is set when R[14] of response register is set to 1 */
        HW_SDHC_EVENT_CQE_EVENT                 =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, CQE_EVENT)),          /**< This status is set if Command Queuing/Crypto event has occurred */
        HW_SDHC_EVENT_ERR_INTERRUPT             =
                (1UL << REG_POS(EMMC, EMMC_NORMAL_INT_STAT_R_REG, ERR_INTERRUPT)),      /**< This status is set if any of the bits in the Error Interrupt Status register is set */
        HW_SDHC_EVENT_ALL_INTERRUPTS            = (0xFFFFUL),                           /**< This is used to enable/disable all interrupts */

        // Software defined events
        HW_SDHC_EVENT_BUF_RD_ENABLE_TIMEOUT     = (1UL << 16UL),                        /**< This bit is set upon a buffer read enable timeout */
        HW_SDHC_EVENT_BUF_WR_ENABLE_TIMEOUT     = (1UL << 17UL),                        /**< This bit is set upon a buffer write enable timeout */
        HW_SDHC_EVENT_ADMA2_ERROR               = (1UL << 18UL),                        /**< This bit is set upon a ADMA2 error */
        HW_SDHC_EVENT_ERROR_RECOVERY_ERROR      = (1UL << 19UL),                        /**< This bit is set upon a general error-recovery error */
        HW_SDHC_EVENT_NON_RECOVERABLE_ERROR     = (1UL << 20UL),                        /**< This bit is set upon a non-recoverable error-recovery error */
} HW_SDHC_EVENT;

/**
 * \brief Current state of card status
 *
 * Response R1 (normal response command) contains the Card Status which is coded in 32-bits.
 * The Card Status has a 4-bit field which is the Current State of the card, coded as follows.
 *
 */
typedef enum {
        HW_SDHC_CARD_STATUS_CURRENT_STATE_IDLE,         /**< Card state is Idle */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_READY,        /**< Card state is Ready */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_IDENT,        /**< Card state is Identification */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY,         /**< Card state is Stand-by */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_TRAN,         /**< Card state is Transfer */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_DATA,         /**< Card state is Data-sending */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_RCV,          /**< Card state is Receive-data */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_PRG,          /**< Card state is Programming */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_DIS,          /**< Card state is Disconnect */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_BTST,         /**< Card state is Bus test */
        HW_SDHC_CARD_STATUS_CURRENT_STATE_SLP,          /**< Card state is Sleep */
} HW_SDHC_CARD_STATUS_CURRENT_STATE;

/**
 * \brief SD Bus Voltage Select for VDD1
 */
typedef enum {
        HW_SDHC_SD_BUS_VOL_VDD1_RSVD0 = 0x0UL,            /**< SD Bus Voltage: Reserved */
        HW_SDHC_SD_BUS_VOL_VDD1_1V8 = 0x5UL,              /**< SD Bus Voltage: 1.8V (Typical) for Embedded */
        HW_SDHC_SD_BUS_VOL_VDD1_3V0 = 0x6UL,              /**< SD Bus Voltage: 3.0V (Typical) */
        HW_SDHC_SD_BUS_VOL_VDD1_3V3 = 0x7UL,              /**< SD Bus Voltage: 3.3V (Typical) */
} HW_SDHC_SD_PWR_CTRL_R_BUS_VOL_VDD1;

/**
 * \brief eMMC Bus Voltage Select for VDD
 */
typedef enum {
        HW_SDHC_EMMC_BUS_VOL_VDD1_RSVD0 = 0x0UL,          /**< SD Bus Voltage: Reserved */
        HW_SDHC_EMMC_BUS_VOL_VDD1_1V2 = 0x5UL,            /**< SD Bus Voltage: 1.2V (Typical) */
        HW_SDHC_EMMC_BUS_VOL_VDD1_1V8 = 0x6UL,            /**< SD Bus Voltage: 1.8V (Typical) */
        HW_SDHC_EMMC_BUS_VOL_VDD1_3V3 = 0x7UL,            /**< SD Bus Voltage: 3.3V (Typical) */
} HW_SDHC_EMMC_PWR_CTRL_R_BUS_VOL_VDD1;

#if (HW_SDHC_HC_CLOCK_GENERATOR_SUPPORTED == 1)
/**
 * \brief This bit is used to select the clock generator mode in the Host Controller
 */
typedef enum {
        HW_SDHC_CLK_GEN_SELECT_DIVIDED_CLK_MODE,        /**< Clock Generator Select: Divided Clock Mode */
        HW_SDHC_CLK_GEN_SELECT_PROG_CLK_MODE,           /**< Clock Generator Select: Programmable Clock Mode */
} HW_SDHC_CLK_CTRL_R_CLK_GEN_SELECT;
#endif

/**
 * \brief Define the command types
 */
typedef enum {
        HW_SDHC_CMD_TYPE_NORMAL,                        /**< Command Type: Normal */
        HW_SDHC_CMD_TYPE_SUSPEND,                       /**< Command Type: Suspend */
        HW_SDHC_CMD_TYPE_RESUME,                        /**< Command Type: Resume */
        HW_SDHC_CMD_TYPE_ABORT,                         /**< Command Type: Abort */
} HW_SDHC_CMD_R_CMD_TYPE;

/**
 * \brief Distinguish between main and sub-command types
 */
typedef enum {
        HW_SDHC_SUB_CMD_FLAG_MAIN,                      /**< Sub Command Flag: Main */
        HW_SDHC_SUB_CMD_FLAG_SUB,                       /**< Sub Command Flag: Sub-command */
} HW_SDHC_CMD_R_SUB_CMD_FLAG;

/**
 * \brief Define the types of response expected from the card.
 */
typedef enum {
        HW_SDHC_RESP_TYPE_SELECT_NO_RESP,               /**< Card Response Type Select : No response */
        HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_136,          /**< Card Response Type Select : Response 136 bits */
        HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48,           /**< Card Response Type Select : Response 48 bits */
        HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48B,          /**< Card Response Type Select : Response 48 bits; Check Busy after response */
} HW_SDHC_CMD_R_RESP_TYPE_SELECT;

/**
 * \brief UHS bus speed modes
 */
typedef enum {
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_SDR12,           /**< UHS Mode (SD/UHS-II mode only): SDR12 */
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_SDR25,           /**< UHS Mode (SD/UHS-II mode only): SDR25 */
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_SDR50,           /**< UHS Mode (SD/UHS-II mode only): SDR50 */
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_SDR104,          /**< UHS Mode (SD/UHS-II mode only): SDR104 */
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_DDR50,           /**< UHS Mode (SD/UHS-II mode only): DDR50 */
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_RES1,            /**< UHS Mode (SD/UHS-II mode only): Reserved */
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_RES2,            /**< UHS Mode (SD/UHS-II mode only): Reserved */
        HW_SDHC_UHS_BUS_SPEED_MODE_SEL_UHS2,            /**< UHS Mode (SD/UHS-II mode only): UHS2 */
} HW_SDHC_HOST_CTRL2_R_UHS_BUS_SPEED_MODE_SEL;

/**
 * \brief eMMC bus speed modes
 */
typedef enum {
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_LEGACY,         /**< eMMC Mode: Legacy */
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS_SDR,         /**< eMMC Mode: High Speed SDR */
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_RES1,           /**< eMMC Mode: Reserved */
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS200,          /**< eMMC Mode: HS200 */
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS_DDR,         /**< eMMC Mode: High Speed DDR */
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_RES2,           /**< eMMC Mode: Reserved */
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_RES3,           /**< eMMC Mode: Reserved */
        HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS400,          /**< eMMC Mode: HS400 */
} HW_SDHC_HOST_CTRL2_R_EMMC_BUS_SPEED_MODE_SEL;

/**
 * \brief DMA modes
 */
typedef enum {
        HW_SDHC_DMA_SEL_SDMA,                           /**< DMA Select: SDMA */
        HW_SDHC_DMA_SEL_RES,                            /**< DMA Select: Reserved bit*/
        HW_SDHC_DMA_SEL_ADMA2,                          /**< DMA Select: ADMA2 */
        HW_SDHC_DMA_SEL_SDMA2_3,                        /**< DMA Select: ADMA2 or ADMA3 */
} HW_SDHC_HOST_CTRL1_R_DMA_SEL;

/**
 * \brief SDMA Buffer Boundary
 */
typedef enum {
        HW_SDHC_SDMA_BUF_BDARY_4KB,                     /**< 4K bytes SDMA Buffer Boundary */
        HW_SDHC_SDMA_BUF_BDARY_8KB,                     /**< 8K bytes SDMA Buffer Boundary */
        HW_SDHC_SDMA_BUF_BDARY_16KB,                    /**< 16K bytes SDMA Buffer Boundary */
        HW_SDHC_SDMA_BUF_BDARY_32KB,                    /**< 32K bytes SDMA Buffer Boundary */
        HW_SDHC_SDMA_BUF_BDARY_64KB,                    /**< 64K bytes SDMA Buffer Boundary */
        HW_SDHC_SDMA_BUF_BDARY_128KB,                   /**< 128K bytes SDMA Buffer Boundary */
        HW_SDHC_SDMA_BUF_BDARY_256KB,                   /**< 256K bytes SDMA Buffer Boundary */
        HW_SDHC_SDMA_BUF_BDARY_512KB,                   /**< 512K bytes SDMA Buffer Boundary */
} HW_SDHC_BLOCKSIZE_R_SDMA_BUF_BDARY;

/**
 * \brief ADMA2 Descriptor table actions enumeration
 */
typedef enum {
        HW_SDHC_ADMA2_ACT_NOP,                          /**< No Operation: do not execute current line and goto next one */
        HW_SDHC_ADMA2_ACT_RSVD,                         /**< Same as NOP: do not execute current line and goto next one */
        HW_SDHC_ADMA2_ACT_TRAN,                         /**< Transfer data of current descriptor line */
        HW_SDHC_ADMA2_ACT_LINK,                         /**< Link (pointer) to another descriptor line */
} HW_SDHC_ADMA2_ACT;

/**
 * \brief ADMA2 Descriptor table length mode enumeration
 */
typedef enum {
        HW_SDHC_ADMA2_LEN_MODE_16BIT,                   /**< 16-bit ADMA2 Data Length Mode */
        HW_SDHC_ADMA2_LEN_MODE_26BIT,                   /**< 26-bit ADMA2 Data Length Mode */
} HW_SDHC_ADMA2_LEN_MODE;

/**
 * \brief Host Controller output driver in 1.8V signaling UHS-I/eMMC speed modes
 */
typedef enum {
        HW_SDHC_DRV_STRENGTH_SEL_TYPEB,                 /**< Driver TYPEB is selected */
        HW_SDHC_DRV_STRENGTH_SEL_TYPEA,                 /**< Driver TYPEA is selected */
        HW_SDHC_DRV_STRENGTH_SEL_TYPEC,                 /**< Driver TYPEC is selected */
        HW_SDHC_DRV_STRENGTH_SEL_TYPED,                 /**< Driver TYPED is selected */
} HW_SDHC_HOST_CTRL2_R_DRV_STRENGTH_SEL;

/**
 * \brief Determines use of Auto Command functions
 */
typedef enum {
        HW_SDHC_AUTO_CMD_ENABLE_DISABLED,               /**< Auto Command Disabled */
        HW_SDHC_AUTO_CMD_ENABLE_CMD12,                  /**< Auto CMD12 Enabled */
        HW_SDHC_AUTO_CMD_ENABLE_CMD23,                  /**< Auto CMD23 Enabled */
        HW_SDHC_AUTO_CMD_ENABLE_AUTO_SEL,               /**< Auto CMD Auto Select */
} HW_SDHC_XFER_MODE_R_AUTO_CMD_ENABLE;

/**
 * \brief Data transfer direction
 */
typedef enum {
        HW_SDHC_DATA_XFER_DIR_WRITE,                    /**< Data Transfer Direction Select: Write Host to Card */
        HW_SDHC_DATA_XFER_DIR_READ,                     /**< Data Transfer Direction Select: Read Card to Host */
} HW_SDHC_XFER_MODE_R_DATA_XFER_DIR;

/**
 * \brief HC LLD state saved in context data
 */
typedef enum {
        HW_SDHC_STATE_FREE,                             /**< Host Controller State: Free */
        HW_SDHC_STATE_IDLE,                             /**< Host Controller State: Idle */
        HW_SDHC_STATE_WAIT_CMD_COMPLETE,                /**< Host Controller State: Wait command complete event */
        HW_SDHC_STATE_WAIT_DATA_XFER_COMPLETE,          /**< Host Controller State: Wait data transfer complete event */
} HW_SDHC_STATE;

/**
 * \brief Enumeration used in bus configuration
 */
typedef enum {
        HW_SDHC_BUS_WIDTH_1_BIT,                        /**< The 1-bit SDR mode data transfer width */
        HW_SDHC_BUS_WIDTH_4_BIT,                        /**< The 4-bit SDR mode data transfer width */
        HW_SDHC_BUS_WIDTH_8_BIT,                        /**< The 8-bit SDR mode data transfer width */
        HW_SDHC_BUS_WIDTH_RSVD1,                        /**< Reserved value */
        HW_SDHC_BUS_WIDTH_RSVD2,                        /**< Reserved value */
#if (HW_SDHC_SUPPORT_DDR == 1)
        HW_SDHC_BUS_WIDTH_4_BIT_DDR,                    /**< The 4-bit DDR mode data transfer width */
        HW_SDHC_BUS_WIDTH_8_BIT_DDR,                    /**< The 8-bit DDR mode data transfer width */
#endif
} HW_SDHC_BUS_WIDTH;

/**
 * \brief CMD6 access type enumeration
 */
typedef enum {
        HW_SDHC_CMD6_ACCESS_CMD_SET,                    /**< CMD6 Access Mode: Command Set. */
        HW_SDHC_CMD6_ACCESS_SET_BITS,                   /**< CMD6 Access Mode: Set Bits. */
        HW_SDHC_CMD6_ACCESS_CLR_BITS,                   /**< CMD6 Access Mode: Clear Bits. */
        HW_SDHC_CMD6_ACCESS_WRITE_BYTE,                 /**< CMD6 Access Mode: Write Byte. */
} HW_SDHC_CMD6_ACCESS;

/**
 * \brief CMD_R: command index codes
 */
typedef enum {
        HW_SDHC_CMD_INDEX_CMD0,                         /**< GO_IDLE_STATE, GO_PRE_IDLE_STATE, BOOT_INITIATION */
        HW_SDHC_CMD_INDEX_CMD1,                         /**< SEND_OP_COND */
        HW_SDHC_CMD_INDEX_CMD2,                         /**< ALL_SEND_CID */
        HW_SDHC_CMD_INDEX_CMD3,                         /**< SET_RELATIVE_ADDR */
        HW_SDHC_CMD_INDEX_CMD4,                         /**< SET_DSR */
        HW_SDHC_CMD_INDEX_CMD5,                         /**< SLEEP_AWAKE */
        HW_SDHC_CMD_INDEX_CMD6,                         /**< SWITCH */
        HW_SDHC_CMD_INDEX_CMD7,                         /**< SELECT/DESELECT_CARD */
        HW_SDHC_CMD_INDEX_CMD8,                         /**< SEND_EXT_CSD */
        HW_SDHC_CMD_INDEX_CMD9,                         /**< SEND_CSD */

        HW_SDHC_CMD_INDEX_CMD10,                        /**< SEND_CID */
        HW_SDHC_CMD_INDEX_CMD11,                        /**< READ_DAT_UNTIL_STOP */
        HW_SDHC_CMD_INDEX_CMD12,                        /**< STOP_TRANSMISSION */
        HW_SDHC_CMD_INDEX_CMD13,                        /**< SEND_STATUS */
        HW_SDHC_CMD_INDEX_CMD14,                        /**< BUSTEST_R */
        HW_SDHC_CMD_INDEX_CMD15,                        /**< GO_INACTIVE_STATE */
        HW_SDHC_CMD_INDEX_CMD16,                        /**< SET_BLOCKLEN */
        HW_SDHC_CMD_INDEX_CMD17,                        /**< READ_SINGLE_BLOCK */
        HW_SDHC_CMD_INDEX_CMD18,                        /**< READ_MULTIPLE_BLOCK */
        HW_SDHC_CMD_INDEX_CMD19,                        /**< BUSTEST_W */

        HW_SDHC_CMD_INDEX_CMD20,                        /**< WRITE_DAT_UNTIL_STOP */
        HW_SDHC_CMD_INDEX_CMD21,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD22,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD23,                        /**< SET_BLOCK_COUNT */
        HW_SDHC_CMD_INDEX_CMD24,                        /**< WRITE_SINGLE_BLOCK */
        HW_SDHC_CMD_INDEX_CMD25,                        /**< WRITE_MULTIPLE_BLOCK */
        HW_SDHC_CMD_INDEX_CMD26,                        /**< PROGRAM_CID */
        HW_SDHC_CMD_INDEX_CMD27,                        /**< PROGRAM_CSD */
        HW_SDHC_CMD_INDEX_CMD28,                        /**< SET_WRITE_PROT */
        HW_SDHC_CMD_INDEX_CMD29,                        /**< CLR_WRITE_PROT */

        HW_SDHC_CMD_INDEX_CMD30,                        /**< SEND_WRITE_PROT */
        HW_SDHC_CMD_INDEX_CMD31,                        /**< SEND_WRITE_PROT_TYPE */
        HW_SDHC_CMD_INDEX_CMD32,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD33,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD34,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD35,                        /**< ERASE_GROUP_START */
        HW_SDHC_CMD_INDEX_CMD36,                        /**< ERASE_GROUP_END */
        HW_SDHC_CMD_INDEX_CMD37,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD38,                        /**< ERASE */
        HW_SDHC_CMD_INDEX_CMD39,                        /**< FAST_IO */

        HW_SDHC_CMD_INDEX_CMD40,                        /**< GO_IRQ_STATE */
        HW_SDHC_CMD_INDEX_CMD41,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD42,                        /**< LOCK_UNLOCK */
        HW_SDHC_CMD_INDEX_CMD43,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD44,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD45,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD46,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD47,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD48,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD49,                        /**< Reserved */

        HW_SDHC_CMD_INDEX_CMD50,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD51,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD52,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD53,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD54,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD55,                        /**< APP_CMD */
        HW_SDHC_CMD_INDEX_CMD56,                        /**< GEN_CMD */
        HW_SDHC_CMD_INDEX_CMD57,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD58,                        /**< Reserved */
        HW_SDHC_CMD_INDEX_CMD59,                        /**< Reserved */

        HW_SDHC_CMD_INDEX_CMD60,                        /**< Reserved for manufacturer */
        HW_SDHC_CMD_INDEX_CMD61,                        /**< Reserved for manufacturer */
        HW_SDHC_CMD_INDEX_CMD62,                        /**< Reserved for manufacturer */
        HW_SDHC_CMD_INDEX_CMD63,                        /**< Reserved for manufacturer */

        HW_SDHC_CMD_INDEX_MAX_LIMIT,
} HW_SDHC_CMD_R_CMD_INDEX;

/**
 * \brief Define R1 or R5 as a response type when the Response Error Check is selected
 */
typedef enum {
        HW_SDHC_RESP_TYPE_R1_MEMORY,                    /**< Response Type: R1 (Memory) */
} HW_SDHC_XFER_MODE_R_RESP_TYPE;

/**
 * \brief Timeout Clock Unit
 */
typedef enum {
        HW_SDHC_TOUT_CLK_UNIT_KHZ,                      /**< Timeout Clock Unit in KHz */
        HW_SDHC_TOUT_CLK_UNIT_MHZ,                      /**< Timeout Clock Unit in MHz */
} HW_SDHC_CAPABILITIES1_R_TOUT_CLK_UNIT;

/**
 * \brief Specification Version Number
 */
typedef enum {
        HW_SDHC_SPEC_VERSION_1_00,                      /**< SD Host Controller Specification Version 1.00 */
        HW_SDHC_SPEC_VERSION_2_00,                      /**< SD Host Controller Specification Version 2.00 */
        HW_SDHC_SPEC_VERSION_3_00,                      /**< SD Host Controller Specification Version 3.00 */
        HW_SDHC_SPEC_VERSION_4_00,                      /**< SD Host Controller Specification Version 4.00 */
        HW_SDHC_SPEC_VERSION_4_10,                      /**< SD Host Controller Specification Version 4.10 */
        HW_SDHC_SPEC_VERSION_4_20,                      /**< SD Host Controller Specification Version 4.20 */
} HW_SDHC_HOST_CNTRL_VERS_R_SPEC_VERSION_NUM;

/**
 * \brief Data transfer bus width
 */
typedef enum {
        HW_SDHC_DAT_XFER_WIDTH_1BIT,                    /**< Data Transfer Width: 1-bit mode */
        HW_SDHC_DAT_XFER_WIDTH_4BIT,                    /**< Data Transfer Width: 4-bit mode */
} HW_SDHC_HOST_CTRL1_R_DAT_XFER_WIDTH;

/**
 * \brief Card Detect signal selection
 */
typedef enum {
        HW_SDHC_CARD_DETECT_SIG_SEL_SDCD_PIN,           /**< SDCD# (card_detect_n signal) is selected (for normal use) */
        HW_SDHC_CARD_DETECT_SIG_SEL_CARD_DT_TEST_LEVEL, /**< Card Detect Test Level is selected (for test purpose) */
} HW_SDHC_HOST_CTRL1_R_CARD_DETECT_SIG_SEL;

/**
 * \brief Card Detect test level
 */
typedef enum {
        HW_SDHC_CARD_DETECT_TEST_LVL_NO_CARD,           /**< No Card */
        HW_SDHC_CARD_DETECT_TEST_LVL_CARD_IN,           /**< Card Inserted */
} HW_SDHC_HOST_CTRL1_R_CARD_DETECT_TEST_LVL;

/**
 * \brief Extended Data Transfer bus width
 */
typedef enum {
        HW_SDHC_EXT_DAT_XFER_DEFAULT,                   /**< Embedded device Bus Width is selected by the Data Transfer Width */
        HW_SDHC_EXT_DAT_XFER_8BIT,                      /**< Embedded device Bus Width is extended to 8-bits */
} HW_SDHC_HOST_CTRL1_R_EXT_DAT_XFER;

/**
 * brief Max Block Length
 */
typedef enum {
        HW_SDHC_CAP1_R_MAX_BLK_LEN_512B,                /**< Maximum block length: 512 Bytes */
        HW_SDHC_CAP1_R_MAX_BLK_LEN_1024B,               /**< Maximum block length: 1024 Bytes */
        HW_SDHC_CAP1_R_MAX_BLK_LEN_2048B,               /**< Maximum block length: 2048 Bytes */
        HW_SDHC_CAP1_R_MAX_BLK_LEN_RESVD,               /**< Maximum block length: reserved */
} HW_SDHC_CAPABILITIES1_R_MAX_BLK_LEN;


/**
 * \brief Data transfer Abort methods
 */
typedef enum {
        HW_SDHC_ABORT_METHOD_SYNC,                      /**< Synchronous abort transaction */
        HW_SDHC_ABORT_METHOD_ASYNC,                     /**< Asynchronous abort transaction */
} HW_SDHC_ABORT_METHOD;

/**
 * \brief Program CID or CSD register
 */
typedef enum {
        HW_SDHC_PROGRAM_CID,                            /**< Program CID register */
        HW_SDHC_PROGRAM_CSD,                            /**< Program CSD register */
} HW_SDHC_PROGRAM_CID_CSD;

/**
 * \brief Write protection type
 */
typedef enum {
        HW_SDHC_WRITE_PROTECTION_TYPE_NONE,             /**< Write protection group not protected */
        HW_SDHC_WRITE_PROTECTION_TYPE_TEMP,             /**< Write protection group is protected by temporary write protection */
        HW_SDHC_WRITE_PROTECTION_TYPE_PWRON,            /**< Write protection group is protected by power-on write protection */
        HW_SDHC_WRITE_PROTECTION_TYPE_PERM,             /**< Write protection group is protected by permanent write protection */
} HW_SDHC_WRITE_PROTECTION_TYPE;

/**
 * \brief Erase command (CMD38) Valid arguments
 */
typedef enum {
        HW_SDHC_CMD38_ARG_ERASE =               0x00000000UL,   /**< CMD38 argument: Erase */
        HW_SDHC_CMD38_ARG_TRIM =                0x00000001UL,   /**< CMD38 argument: Trim */
        HW_SDHC_CMD38_ARG_SECURE_ERASE =        0x80000000UL,   /**< CMD38 argument: Secure Erase */
        HW_SDHC_CMD38_ARG_SECURE_TRIM_STEP_1 =  0x80000001UL,   /**< CMD38 argument: Secure Trim Step 1 */
        HW_SDHC_CMD38_ARG_SECURE_TRIM_STEP_2 =  0x80008000UL,   /**< CMD38 argument: Secure Trim Step 2 */
} HW_SDHC_CMD38_ARG;

/**
 * \brief Lock/unlock command (CMD42) Valid arguments
 */
typedef enum {
        HW_SDHC_CMD42_CMD_UNLOCK =              (uint8_t) 0x00,         /**< CMD42 command byte: Unlock card */
        HW_SDHC_CMD42_CMD_SET_PWD =             (uint8_t) BIT0,         /**< CMD42 command byte: Set Password */
        HW_SDHC_CMD42_CMD_CLR_PWD =             (uint8_t) BIT1,         /**< CMD42 command byte: Clear Password */
        HW_SDHC_CMD42_CMD_LOCK =                (uint8_t) BIT2,         /**< CMD42 command byte: Lock card */
        HW_SDHC_CMD42_CMD_ERASE =               (uint8_t) BIT3,         /**< CMD42 command byte: Force erase */
} HW_SDHC_CMD42_CMD;

/*
 *****************************************************************************************
 *
 * Struct's.
 *
 *****************************************************************************************
 */

/**
 * \brief SD Host Write/Read structure
 *
 */
typedef struct {
        HW_SDHC_XFER_MODE_R_DATA_XFER_DIR       xfer_dir;               /**< Set the transaction direction */
        bool                                    dma_en;                 /**< Enable DMA for the transaction */
        bool                                    intr_en;                /**< Enable Interrupt (non-blocking) mode for the transaction */
        HW_SDHC_HOST_CTRL1_R_DMA_SEL            dma_type;               /**< Set DMA Type: SDMA or ADMA */
        bool                                    use_32bit_counter;      /**< If SDMA then SDMASA_R is used for block counter.
                                                                             SDIO: always false, since blk_cnt < 512 */
        uint8_t*                                data;                   /**< The pointer to data on system memory */
        uint32_t                                address;                /**< eMMC: sector address to Write/Read data on the card
                                                                             SDIO: register address */
        uint16_t                                block_size;             /**< The SDIO/eMMC memory card block size */
        uint32_t                                block_cnt;              /**< The number of blocks to Write/Read */
        HW_SDHC_XFER_MODE_R_AUTO_CMD_ENABLE     auto_command;           /**< Selects which auto commands are used if any. SDIO: always HW_SDHC_AUTO_CMD_ENABLE_DISABLED */
        uint32_t                                tout_cnt_time;          /**< Data Timeout Counter in ms or us depending on the value of EMMC_CAPABILITIES1_R_REG:TOUT_CLK_UNIT
                                                                         *   Set EMMC_TOUT_CTRL_R_REG:TOUT_CNT, which is calculated from tout_cnt_time */
        uint32_t                                xfer_tout_ms;           /**< Data transfer timeout (read/write/erase) as calculated by the card registers or determined by the user, in msec */
#if (dg_configUSE_HW_EMMC == 1)
        bool                                    set_blk_len;            /**< Set block length using CMD16 */
        bool                                    emmc_reliable_write_en; /**< Enables the reliable write in CMD23 */
        bool                                    bus_testing;            /**< Enable bus testing procedure */
#endif
        HW_SDHC_BLOCKSIZE_R_SDMA_BUF_BDARY      page_bdary;             /**< Page Boundary of system memory */
        HW_SDHC_ADMA2_LEN_MODE                  adma2_len_mode;         /**< ADMA2 data length mode: 16-bit or 26-bit */
} hw_sdhc_data_transfer_config_t;

/**
 * \brief SD Host command configuration structure
 *
 */
typedef struct {
        uint32_t                        cmd_index;              /**< The index of the command. */
        uint32_t                        cmd_arg;                /**< The argument for the command. */
        HW_SDHC_CMD_R_CMD_TYPE          cmd_type;               /**< The command type. */
        HW_SDHC_CMD_R_SUB_CMD_FLAG      sub_cmd_flag;           /**< Set sub-command flag, main or sub-command. */
        bool                            crc_check_en;           /**< Enables the CRC check on the response. */
        bool                            idx_check_en;           /**< Checks the index of the response. */
        HW_SDHC_CMD_R_RESP_TYPE_SELECT  resp_type;              /**< The response type. */
        bool                            read_resp;              /**< true: Read response registers.
                                                                 *  If resp_type is RESP_LEN_136 then read 4 response registers,
                                                                 *  else read only one. */
        bool                            data_present;           /**< true: Data transferred using the DAT line,
                                                                 *  false: Commands use the CMD line only. */
        bool                            wait_cmd_complete;      /**< Wait for CMD_COMPLETE signal. */
        uint32_t                        cmd_complete_delay;     /**< Add a delay before reading CMD_COMPLETE signal. */
        bool                            check_errors;           /**< Check card status for errors. */
        bool                            wait_for_busy;          /**< In case of R1b, wait for busy. */
        uint32_t                        busy_tout_ms;           /**< Busy timeout in msec. */
} hw_sdhc_cmd_config_t;

/**
 * \brief SD Host Setup configuration structure
 *
 */
typedef struct {
        uint8_t                         bus_vol_vdd1;   /**< Set SD Bus Voltage Select for VDD1
                                                         *   or eMMC Bus Voltage Select for VDD */
        uint8_t                         tout_cnt;       /**< Set Data Timeout Counter Value at EMMC_TOUT_CTRL_R_REG:TOUT_CNT */
        uint32_t                        tout;           /**< Set Data Timeout Counter in ms or us depending on the value of EMMC_CAPABILITIES1_R_REG:TOUT_CLK_UNIT
                                                         *   This field is used if the user sets tout_cnt=0xFF
                                                         *   The field tout_cnt, used to set EMMC_TOUT_CTRL_R_REG:TOUT_CNT, is calculated from tout */
} hw_sdhc_hc_setup_config_t;

/**
 * \brief SD Host Bus configuration structure
 *
 */
typedef struct {
        uint32_t                                bus_speed;      /**< Set bus speed in Hz */
        uint8_t                                 speed_mode;     /**< Set bus speed mode */
        HW_SDHC_BUS_WIDTH                       bus_width;      /**< Set bus width */
        HW_SDHC_HOST_CTRL2_R_DRV_STRENGTH_SEL   drv_strength;   /**< Set bus drive strength */

        bool                                    dsr_req;        /**< Request DSR configuration */
        uint16_t                                dsr;            /**< DSR value */
} hw_sdhc_bus_config_t;

/**
 * \brief PDCTRL REG configuration structure
 *
 */
typedef struct {
        uint8_t                         clk_div;                /**< CLK_PDCTRL_REG: Set clock divider value */
        uint8_t                         inv_rx_clk:1;           /**< CLK_PDCTRL_REG: Inverts the clock in the RX path, cascaded with INV_TX_CLK */
        uint8_t                         inv_tx_clk:1;           /**< CLK_PDCTRL_REG: Inverts the clock in the TX path */
} hw_sdhc_pdctrl_reg_config_t;

/**
 * \brief SD Host configuration structure
 *
 */
typedef struct {
        hw_sdhc_hc_setup_config_t       hc_setup;               /**< Set Host Controller parameters */
        hw_sdhc_bus_config_t            bus_config;             /**< Set bus parameters */
} hw_sdhc_config_t;

/**
 * \brief Structure used to define the length and attributes
 * of the ADMA2 descriptor table in 32-bit Addressing Mode.
 *
 */
typedef __PACKED_STRUCT {
        uint32_t valid:1;                               /**< bit: 0      Indicates validity of a Descriptor Line */
        uint32_t end:1;                                 /**< bit: 1      End of Descriptor. XFER_COMPLETE is set. */
        uint32_t intr:1;                                /**< bit: 2      Generates DMA_INTERRUPT when this line xfer is complete */
        uint32_t act:3;                                 /**< bit: 3..5   Bits-2:0 of operation code */
        uint32_t len_upper:10;                          /**< bit: 6..15  Extended mode (26-bit) data length from ver 4.10 */
        uint32_t len_lower:16;                          /**< bit: 16..31 Extended mode (26-bit) data length from ver 4.10 */
} hw_sdhc_desc_attr_n_len_t;

/**
 * \brief Structure used to define the ADMA2 descriptor table in 32-bit Addressing Mode.
 *
 */
typedef struct {
        hw_sdhc_desc_attr_n_len_t       attr_n_len;     /**< Length and Attribute */
        uint32_t                        addr;           /**< Address 32-bit */
} hw_sdhc_adma_descriptor_table_t;

/**
 * \brief CID - Card Identification Register
 * It is a 128-bit register that contains device identification information
 * used during the eMMC protocol device identification phase.
 *
 * \note: CRC is not included
 *
 */
typedef __PACKED_STRUCT
{
        uint8_t month:4;                /**< Manufacturing date: month */
        uint8_t year:4;                 /**< Manufacturing date: year */
        uint32_t psn;                   /**< Product serial number */
        uint8_t prv;                    /**< Product revision */
        char pnm[6];                    /**< Product name */
        uint8_t oid;                    /**< OEM/Application ID */
        uint8_t cbx:2;                  /**< Card/BGA */
        uint8_t reserved_0:6;           /**< Reserved */
        uint8_t mid;                    /**< Manufacturer ID */
} hw_sdhc_emmc_cid_t;

/**
 * \brief CSD - Card-Specific Data Register
 * It is a 128-bit register that provides information on how to access
 * the contents stored in eMMC.
 *
 * \note: CRC is not included
 *
 */
typedef __PACKED_STRUCT
{
        uint8_t ecc:2;                          /**< ECC code */
        uint8_t file_format:2;                  /**< File format */
        uint8_t tmp_write_protect:1;            /**< Temporary write protection */
        uint8_t perm_write_protect:1;           /**< Permanent write protection */
        uint8_t copy:1;                         /**< Copy flag (OTP) */
        uint8_t file_format_grp:1;              /**< File format group */
        uint16_t content_prot_app:1;            /**< Content protection application */
        uint16_t reserved_0:4;                  /**< Reserved */
        uint16_t write_bl_partial:1;            /**< Partial blocks for write allowed */
        uint16_t write_bl_len:4;                /**< Max write data block length */
        uint16_t r2w_factor:3;                  /**< Write speed factor */
        uint16_t default_ecc:2;                 /**< Manufacturer default ECC */
        uint16_t wp_grp_enable:1;               /**< Write protect group enable */
        uint32_t wp_grp_size:5;                 /**< Write protect group size */
        uint32_t erase_grp_mult:5;              /**< Erase group size multiplier */
        uint32_t erase_grp_size:5;              /**< Erase group size */
        uint32_t c_size_mult:3;                 /**< Device size multiplier */
        uint8_t vdd_w_curr_max:3;               /**< Max write current @ VDD max */
        uint8_t vdd_w_curr_min:3;               /**< Max write current @ VDD min */
        uint8_t vdd_r_curr_max:3;               /**< Max read current @ VDD max */
        uint8_t vdd_r_curr_min:3;               /**< Max read current @ VDD min */
        uint16_t c_size:12;                     /**< Device size */
        uint16_t reserved_1:2;                  /**< Reserved */
        uint8_t dsr_imp:1;                      /**< DSR implemented */
        uint8_t read_blk_misalign:1;            /**< Read block misalignment */
        uint8_t write_blk_misalign:1;           /**< Write block misalignment */
        uint8_t read_bl_partial:1;              /**< Partial blocks for read allowed */
        uint8_t read_bl_len:4;                  /**< Max read data block length */
        uint16_t ccc:12;                        /**< Card command classes */
        uint8_t tran_speed;                     /**< Max bus clock frequency */
        uint8_t nsac;                           /**< Data read access-time 2 in CLK cycles (NSAC*100) */
        uint8_t taac;                           /**< Data read access-time 1 */
        uint8_t reserved_2:2;                   /**< Reserved */
        uint8_t spec_ver:4;                     /**< System specification version */
        uint8_t csd_structure:2;                /**< CSD structure */
} hw_sdhc_emmc_csd_t;

/**
 * \brief EXT_CSD - Extended Card-Specific Data Register
 *
 * The Extended CSD register defines the card properties and selected modes. It is 512 bytes long.
 * The most significant 320 bytes are the Properties segment, which defines the card capabilities and
 * cannot be modified by the host.
 * The lower 192 bytes are the Modes segment, which defines the configuration the card is working in.
 * These modes can be changed by the host by means of the SWITCH command (CMD6).
 */
typedef __PACKED_STRUCT {
        /* Modes Segment: Bytes 0:191 (Write-Read) */
        uint8_t reserved_133_0[134];
        uint8_t sec_bad_blk_mgmnt;              /**< Bad Block Management mode */
        uint8_t reserved_135;
        uint8_t enh_start_addr[4];              /**< Enhanced User Data Start Address */
        uint8_t enh_size_mult[3];               /**< Enhanced User Data Area Size */
        uint8_t gp_size_mult[12];               /**< General Purpose Partition Size */
        uint8_t partition_setting_completed;    /**< Partition Setting */
        uint8_t partitions_attribute;           /**< Partitions Attribute */
        uint8_t max_enh_size_mult[3];           /**< Max Enhanced Area Size */
        uint8_t partitioning_support;           /**< Partitioning Support */
        uint8_t hpi_mgmt;                       /**< HPI management */
        uint8_t rst_n_function;                 /**< H/W reset function */
        uint8_t bkops_en;                       /**< Enable background operations handshake */
        uint8_t bkops_start;                    /**< Manually start background operations */
        uint8_t reserved_165;
        uint8_t wr_rel_param;                   /**< Write reliability parameter register */
        uint8_t wr_rel_set;                     /**< Write reliability setting register */
        uint8_t rpmb_size_mult;                 /**< RPMB Size */
        uint8_t fw_config;                      /**< FW configuration */
        uint8_t reserved_170;
        uint8_t user_wp;                        /**< User area write protection register */
        uint8_t reserved_172;
        uint8_t boot_wp;                        /**< Boot area write protection register */
        uint8_t reserved_174;
        uint8_t erase_group_def;                /**< High-density erase group definition */
        uint8_t reserved_176;
        uint8_t boot_bus_width;                 /**< Boot bus width */
        uint8_t boot_config_prot;               /**< Boot configuration protection */
        uint8_t partition_config;               /**< Partition configuration */
        uint8_t reserved_180;
        uint8_t erased_mem_cont;                /**< Erased memory content */
        uint8_t reserved_182;
        uint8_t bus_width;                      /**< Bus width mode */
        uint8_t reserved_184;
        uint8_t hs_timing;                      /**< High-speed interface timing */
        uint8_t reserved_186;
        uint8_t power_class;                    /**< Power class */
        uint8_t reserved_188;
        uint8_t cmd_set_rev;                    /**< Command set revision */
        uint8_t reserved_190;
        uint8_t cmd_set;                        /**< Command set */

        /* Properties Segment: Bytes 192:511 (Read-only) */
        uint8_t ext_csd_rev;                    /**< Extended CSD revision */
        uint8_t reserved_193;
        uint8_t csd_structure;                  /**< CSD structure version */
        uint8_t reserved_195;
        uint8_t card_type;                      /**< Card type */
        uint8_t reserved_197;
        uint8_t out_of_interrupt_time;          /**< Out-of-interrupt busy timing */
        uint8_t partition_switch_time;          /**< Partition switching timing */
        uint8_t pwr_cl_52_195;                  /**< Power class for 52MHz at 1.95V */
        uint8_t pwr_cl_26_195;                  /**< Power class for 26MHz at 1.95V */
        uint8_t pwr_cl_52_360;                  /**< Power class for 52MHz at 3.6V */
        uint8_t pwr_cl_26_360;                  /**< Power class for 26MHz at 3.6V */
        uint8_t reserved_204;
        uint8_t min_perf_r_4_26;                /**< Minimum Read Performance for 4bit at 26MHz */
        uint8_t min_perf_w_4_26;                /**< Minimum Write Performance for 4bit at 26MHz */
        uint8_t min_perf_r_8_26_4_52;           /**< Minimum Read Performance for 8bit at 26MHz, for 4bit at 52MHz */
        uint8_t min_perf_w_8_26_4_52;           /**< Minimum Write Performance for 8bit at 26MHz, for 4bit at 52MHz */
        uint8_t min_perf_r_8_52;                /**< Minimum Read Performance for 8bit at 52MHz */
        uint8_t min_perf_w_8_52;                /**< Minimum Write Performance for 8bit at 52MHz */
        uint8_t reserved_211;
        uint32_t sec_count;                     /**< Sector Count */
        uint8_t reserved_216;
        uint8_t s_a_timeout;                    /**< Sleep/awake timeout */
        uint8_t reserved_218;
        uint8_t s_c_vccq;                       /**< Sleep current (VCCQ) */
        uint8_t s_c_vcc;                        /**< Sleep current (VCC) */
        uint8_t hc_wp_grp_size;                 /**< High-capacity write protect group size */
        uint8_t rel_wr_sec_c;                   /**< Reliable write sector count */
        uint8_t erase_timeout_mult;             /**< High-capacity erase timeout */
        uint8_t hc_erase_grp_size;              /**< High-capacity erase unit size */
        uint8_t acc_size;                       /**< Access size */
        uint8_t boot_size_multi;                /**< Boot partition size */
        uint8_t reserved_227;
        uint8_t boot_info;                      /**< Boot information */
        uint8_t sec_trim_mult;                  /**< Secure TRIM Multiplier */
        uint8_t sec_erase_mult;                 /**< Secure Erase Multiplier */
        uint8_t sec_feature_support;            /**< Secure Feature support */
        uint8_t trim_mult;                      /**< TRIM Multiplier */
        uint8_t reserved_233;
        uint8_t min_perf_ddr_r_8_52;            /**< Minimum Read Performance for 8bit at 52MHz in DDR mode */
        uint8_t min_perf_ddr_w_8_52;            /**< Minimum Write Performance for 8bit at 52MHz in DDR mode */
        uint8_t reserved_237_236[2];
        uint8_t pwr_cl_ddr_52_195;              /**< Power class for 52MHz, DDR at 1.95V */
        uint8_t pwr_cl_ddr_52_360;              /**< Power class for 52MHz, DDR at 3.6V */
        uint8_t reserved_240;
        uint8_t ini_timeout_ap;                 /**< 1st initialization time after partitioning */
        uint8_t correctly_prg_sectors_num[4];   /**< Number of correctly programmed sectors */
        uint8_t bkops_status;                   /**< Background operations status */
        uint8_t reserved_501_247[255];
        uint8_t bkops_support;                  /**< Background operations support */
        uint8_t hpi_features;                   /**< HPI features */
        uint8_t s_cmd_set;                      /**< Supported Command Sets */
        uint8_t reserved_511_505[7];
} hw_sdhc_emmc_ext_csd_t;

/**
 * \brief Command SWITCH (CMD6) argument fields
 */
typedef struct
{
        uint8_t cmd_set;                /**< Bits [2:0] Command set values */
        uint8_t value;                  /**< Bits [15:8] Value */
        uint8_t index;                  /**< Bits [23:16] Index */
        HW_SDHC_CMD6_ACCESS access;     /**< Bits [25:24] Access mode */
} hw_sdhc_switch_cmd6_arg_t;

/**
 * \brief Command SWITCH (CMD6) configuration
 */
typedef struct
{
        hw_sdhc_switch_cmd6_arg_t cmd_arg;      /**< Bits [2:0] Command set values */
        uint32_t tout_ms;                       /**< Busy response timeout in ms */
} hw_sdhc_switch_cmd6_config_t;

/**
 * \brief typedef for the data transfer abort implementation function
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
typedef HW_SDHC_STATUS (*hw_sdhc_abort_impl_t)(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Typedef for SDHC interrupt handler
 *
 * \param [in] event            event identifier
 *
 * \note: it is defined before \sa hw_sdhc_context_data_t and after \sa HW_SDHC_EVENT definitions
 */
typedef void (*hw_sdhc_event_callback_t)(HW_SDHC_EVENT event);

/**
 * \brief Structure used for SD Host Controller saved data/context
 *
 */
typedef struct {
        HW_SDHC_STATE                           state;          /**< Current state of the driver */
        uint32_t                                cmd_events;     /**< SDHC command events */
        uint32_t                                card_status;    /**< Last card status */
        uint16_t                                error_int_stat; /**< ERROR_INT_STAT_R */
        uint8_t                                 adma_error;     /**< ADMA_ERR_STAT_R */

        bool                                    data_xfer_cmd;  /**< Issue a data transfer command */
        bool                                    read_resp;      /**< Read card response after sending a command or not */
        uint32_t                                *response;      /**< Card response after sending a command */
        HW_SDHC_CMD_R_RESP_TYPE_SELECT          resp_type;      /**< Type of card response*/

        uint32_t                                bus_speed;      /**< Configured bus speed */
        HW_SDHC_BUS_WIDTH                       bus_width;      /**< Configured bus width */

        bool                                    dma_en;         /**< DMA enable */
        HW_SDHC_HOST_CTRL1_R_DMA_SEL            dma_type;       /**< DMA Type */
        HW_SDHC_XFER_MODE_R_DATA_XFER_DIR       xfer_dir;       /**< Read or Write */

        uint32_t                                *data;          /**< The pointer to write/read data on system memory */
        uint16_t                                block_size;     /**< The eMMC memory card block size */

        hw_sdhc_event_callback_t                cb;             /**< User callback function */
        hw_sdhc_abort_impl_t                    abort_impl;     /**< Abort transfer implementation function */

        uint16_t                                normal_int_stat_mask;   /**< Active, applicable and implemented normal interrupts mask */
} hw_sdhc_context_data_t;


#define HW_SDHC_CLK_DIV_MAX                     (16UL)    /**< CLK_PDCTRL_REG.CLK_DIV max value */

#define HW_SDHC_ADMA2_MAX_DESC_TABLE_LINES      (4UL)     /**< Number of ADMA2 descriptors used for transfer.
                                                             Increasing this value above 4 doesn't improve performance for the usual case
                                                             of SD memory cards (most data transfers are multiples of 512 bytes).
                                                             Note that the current implementation uses only one line. */
#define HW_SDHC_ADMA2_MAX_DATA_LEN_MODE_16BIT_BYTES     (1UL << 16UL) /**< ADMA2 max data length mode: 16-bytes */
#define HW_SDHC_ADMA2_MAX_DATA_LEN_MODE_26BIT_BYTES     (1UL << 26UL) /**< ADMA2 max data length mode: 26-bytes */

/*
 * Timeouts and Delays macros
 */
#define HW_SDHC_DELAY_1MS                               (1000UL)  /**< Delay definition for 1ms, in us */
#define HW_SDHC_DELAY_AFTER_CMD0_USEC                   (100UL)   /**< Delay after CMD0, in us */
#define HW_SDHC_DELAY_VOLTAGE_RAMP_UP_US                (1000UL)  /**< Delay for voltage ramp up, in us */
#define HW_SDHC_TOUT_SEND_OP_COND_CMD1_MS               (1000UL)  /**< Timeout for response from CMD1, in ms */
#define HW_SDHC_TOUT_CMD_INHIBIT_MS                     (3UL)     /**< Timeout for command line to be not inhibited, in ms */
#define HW_SDHC_TOUT_CMD_COMPLETE_MS                    (3UL)     /**< Timeout for command to be completed, in ms */
#define HW_SDHC_TOUT_BUF_RD_READY_MS                    (150UL)   /**< Timeout for buffer read ready, in ms */
#define HW_SDHC_TOUT_BUF_RD_ENABLE_MS                   (2UL)     /**< Timeout for buffer read enable, in ms */
#define HW_SDHC_TOUT_BUF_WR_READY_MS                    (150UL)   /**< Timeout for buffer write ready, in ms */
#define HW_SDHC_TOUT_BUF_WR_ENABLE_MS                   (2UL)     /**< Timeout for buffer write enable, in ms */
#define HW_SDHC_DELAY_ERROR_RECOVERY_WAIT_DAT_LINE_US   (40UL)    /**< Delay waiting data line after error recovery, in us. More than 40usec */
#define HW_SDHC_DELAY_CLR_CARD_INTR_US                  (100UL)   /**< Delay waiting card interrupt to be clear, in us */
#define HW_SDHC_TOUT_INTERNAL_CLK_STABLE_MS             (150UL)   /**< Timeout for internal clock to be stable, in ms */
#define HW_SDHC_TOUT_FORCE_ERASE_MS                     (3UL * 60UL * 1000UL) /**< The duration of the Force Erase command using CMD42 is specified to be a fixed time-out of 3 minutes */


#define HW_SDHC_1KHZ                                    (1000UL)                          /**< Frequency definition of 1kHz, in Hz */
#define HW_SDHC_1MHZ                                    (HW_SDHC_1KHZ * HW_SDHC_1KHZ)   /**< Frequency definition of 1MHz, in Hz */

#define HW_SDHC_TOUT_CNT_OFFSET                         (13UL)            /**< TOUT_CNT: offset value */
#define HW_SDHC_TOUT_CNT_MIN                            (1UL << 13UL)         /**< TOUT_CNT: min value */
#define HW_SDHC_TOUT_CNT_MAX                            (1UL << 27UL)         /**< TOUT_CNT: max value */
#define HW_SDHC_TOUT_CNT_INVALID                        (0xFFUL)          /**< TOUT_CNT: invalid value */

#define HW_SDHC_NORMAL_INT_EN_MASK                      (0x7FFFUL)        /**< NORMAL_INT_EN mask */
#define HW_SDHC_ERROR_INT_EN_MASK                       (0xFFFFUL)        /**< ERROR_INT_EN mask */

#define HW_SDHC_RCA_CMD_ARG_POS                         (16UL)            /**< Position of RCA in command argument */
#define HW_SDHC_DSR_CMD_ARG_POS                         (16UL)            /**< Position of DSR in command argument */

#define HW_SDHC_CMD1_OCR_BUSY_MASK                      (1UL << 31UL)         /**< CMD1: OCR busy mask */
#define HW_SDHC_CMD1_OCR_ACCESS_MODE_SECTOR             (2UL << 29UL)         /**< CMD1: High Capacity access mode (sectors) bit */
#define HW_SDHC_CMD1_OCR_ACCESS_MODE_BYTE               (0UL << 29UL)         /**< CMD1: Low Capacity access mode (bytes) bit */
#define HW_SDHC_CMD1_OCR_2V7_3V6                        (0x1FFUL << 15UL)     /**< CMD1: 2V7 to 3V6 mask */
#define HW_SDHC_CMD1_OCR_1V70_1V95                      (1UL << 7UL)          /**< CMD1: 1V70 to 1V95 mask */
#define HW_SDHC_CMD1_VOLTAGE_WINDOW                     (HW_SDHC_CMD1_OCR_ACCESS_MODE_SECTOR |       \
                                                         HW_SDHC_CMD1_OCR_2V7_3V6 |                  \
                                                         HW_SDHC_CMD1_OCR_1V70_1V95)    /**< CMD1: access mode and full voltage window */

#define HW_SDHC_EMMC_EXT_CSD_BUS_WIDTH_IDX              (183UL)           /**< EXT_CSD: bus width index */
#define HW_SDHC_EMMC_EXT_CSD_HS_TIMING_IDX              (185UL)           /**< EXT_CSD: high-speed (HS) timing index */

#define HW_SDHC_CMD6_ARG_CMD_SET_POS                    (0UL)             /**< CMD6 argument: command set position */
#define HW_SDHC_CMD6_ARG_VALUE_POS                      (8UL)             /**< CMD6 argument: value position */
#define HW_SDHC_CMD6_ARG_INDEX_POS                      (16UL)            /**< CMD6 argument: index position */
#define HW_SDHC_CMD6_ARG_ACCESS_POS                     (24UL)            /**< CMD6 argument: access mode position */

#define HW_SDHC_CMD8_ARGUMENT_VHS_2V7_3V3               (0x0100UL)        /**< CMD8 argument: VHS 2V7 to 3V3 value */
#define HW_SDHC_CMD8_VHS_2V7_3V3_MASK                   (0x0100UL)        /**< CMD8 argument: VHS 2V7 to 3V3 mask */
#define HW_SDHC_CMD8_ARGUMENT_CHECK_PATTERN             (0x00AAUL)        /**< CMD8 argument: check pattern value */
#define HW_SDHC_CMD8_CHECK_PATTERN_MASK                 (0x00FFUL)        /**< CMD8 argument: check pattern mask */

/*
 * Card Status macros
 */
#define HW_SDHC_CARD_STATUS_OUT_OF_RANGE                (1UL << 31UL)         /**< Card status: command argument was out-of-range */
#define HW_SDHC_CARD_STATUS_ADDRESS_MISALIGN            (1UL << 30UL)         /**< Card status: misaligned address error */
#define HW_SDHC_CARD_STATUS_BLOCK_LEN_ERROR             (1UL << 29UL)         /**< Card status: not allowed block length error */
#define HW_SDHC_CARD_STATUS_ERASE_SEQ_ERROR             (1UL << 28UL)         /**< Card status: erase commands sequence error */
#define HW_SDHC_CARD_STATUS_ERASE_PARAM                 (1UL << 27UL)         /**< Card status: invalid selection of erase blocks */
#define HW_SDHC_CARD_STATUS_WP_VIOLATION                (1UL << 26UL)         /**< Card status: write protect violation */
#define HW_SDHC_CARD_STATUS_CARD_IS_LOCKED              (1UL << 25UL)         /**< Card status: card is locked */
#define HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED          (1UL << 24UL)         /**< Card status: sequence or password error at lock/unlock */
#define HW_SDHC_CARD_STATUS_COM_CRC_ERROR               (1UL << 23UL)         /**< Card status: CRC error */
#define HW_SDHC_CARD_STATUS_ILLEGAL_COMMAND             (1UL << 22UL)         /**< Card status: illegal command at current card state */
#define HW_SDHC_CARD_STATUS_CARD_ECC_FAILED             (1UL << 21UL)         /**< Card status: card internal ECC failed to correct the data */
#define HW_SDHC_CARD_STATUS_CC_ERROR                    (1UL << 20UL)         /**< Card status: internal card controller error */
#define HW_SDHC_CARD_STATUS_ERROR                       (1UL << 19UL)         /**< Card status: general or unknown error occurred */
#define HW_SDHC_CARD_STATUS_UNDERRUN                    (1UL << 18UL)         /**< Card status: the card could not sustain data transfer in stream read mode */
#define HW_SDHC_CARD_STATUS_OVERRUN                     (1UL << 17UL)         /**< Card status: the card could not sustain data programming in stream write mode */
#define HW_SDHC_CARD_STATUS_CID_CSD_OVERWRITE           (1UL << 16UL)         /**< Card status: CID/CSD overwrite error */
#define HW_SDHC_CARD_STATUS_WP_ERASE_SKIP               (1UL << 15UL)         /**< Card status: erase was partial due to write protected area */
#define HW_SDHC_CARD_STATUS_RSVD14                      (1UL << 14UL)         /**< Card status: reserved bit */
#define HW_SDHC_CARD_STATUS_ERASE_RESET                 (1UL << 13UL)         /**< Card status: erase sequence was cleared */
#define HW_SDHC_CARD_STATUS_CURRENT_STATE_POS           (9UL)             /**< Card status: current state position */
#define HW_SDHC_CARD_STATUS_CURRENT_STATE_MASK          (0X0FUL)          /**< Card status: current state mask */
#define HW_SDHC_CARD_STATUS_READY_FOR_DATA              (1UL << 8UL)          /**< Card status: buffer is empty and ready for data */
#define HW_SDHC_CARD_STATUS_SWITCH_ERROR                (1UL << 7UL)          /**< Card status: SWITCH command error, the card did not switch to the expected mode */
#define HW_SDHC_CARD_STATUS_URGENT_BKOPS                (1UL << 6UL)          /**< Card status: if set, device needs to perform background operations urgently */
#define HW_SDHC_CARD_STATUS_APP_CMD                     (1UL << 5UL)          /**< Card status: an application command (ACMD) is expected */
#define HW_SDHC_CARD_STATUS_RSVD4_0                     (0X1FUL)          /**< Card status: reserved bits */

#define HW_SDHC_CARD_STATUS_BASIC_ERRORS                (HW_SDHC_CARD_STATUS_ERROR | \
                                                         HW_SDHC_CARD_STATUS_CC_ERROR | \
                                                         HW_SDHC_CARD_STATUS_ILLEGAL_COMMAND | \
                                                         HW_SDHC_CARD_STATUS_COM_CRC_ERROR | \
                                                         HW_SDHC_CARD_STATUS_WP_VIOLATION | \
                                                         HW_SDHC_CARD_STATUS_BLOCK_LEN_ERROR | \
                                                         HW_SDHC_CARD_STATUS_ADDRESS_ERROR | \
                                                         HW_SDHC_CARD_STATUS_OUT_OF_RANGE)      /**< Card status: basic errors mask */
#define HW_SDHC_CARD_STATUS_ERRORS_MASK                 (HW_SDHC_CARD_STATUS_OUT_OF_RANGE                | \
                                                         HW_SDHC_CARD_STATUS_ADDRESS_MISALIGN            | \
                                                         HW_SDHC_CARD_STATUS_BLOCK_LEN_ERROR             | \
                                                         HW_SDHC_CARD_STATUS_ERASE_SEQ_ERROR             | \
                                                         HW_SDHC_CARD_STATUS_ERASE_PARAM                 | \
                                                         HW_SDHC_CARD_STATUS_WP_VIOLATION                | \
                                                         HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED          | \
                                                         HW_SDHC_CARD_STATUS_COM_CRC_ERROR               | \
                                                         HW_SDHC_CARD_STATUS_ILLEGAL_COMMAND             | \
                                                         HW_SDHC_CARD_STATUS_CARD_ECC_FAILED             | \
                                                         HW_SDHC_CARD_STATUS_CC_ERROR                    | \
                                                         HW_SDHC_CARD_STATUS_ERROR                       | \
                                                         HW_SDHC_CARD_STATUS_CID_CSD_OVERWRITE           | \
                                                         HW_SDHC_CARD_STATUS_WP_ERASE_SKIP               | \
                                                         HW_SDHC_CARD_STATUS_ERASE_RESET                 | \
                                                         HW_SDHC_CARD_STATUS_SWITCH_ERROR)

#define HW_SDHC_CARD_STATUS_ERASE_ERRORS                (HW_SDHC_CARD_STATUS_OUT_OF_RANGE | \
                                                         HW_SDHC_CARD_STATUS_ERASE_SEQ_ERROR | \
                                                         HW_SDHC_CARD_STATUS_ERASE_PARAM | \
                                                         HW_SDHC_CARD_STATUS_ERASE_RESET)       /**< Card status: erase commands errors */

#define HW_SDHC_PAGE_BDARY_BYTES_4K                     (4UL * 1024UL)      /**< SDMA page boundary 4K, Bytes */

#define HW_SDHC_FREQ_SEL_POS                            (0UL)             /**< CLK_CTRL_R: FREQ_SEL position */
#define HW_SDHC_FREQ_SEL_MASK                           (0x00FFUL)        /**< CLK_CTRL_R: FREQ_SEL mask */
#define HW_SDHC_UPPER_FREQ_SEL_POS                      (8UL)             /**< CLK_CTRL_R: UPPER_FREQ_SEL position */
#define HW_SDHC_UPPER_FREQ_SEL_MASK                     (0x0003UL)        /**< CLK_CTRL_R: UPPER_FREQ_SEL mask */

#define HW_SDHC_CID_SIZE                                (16UL)            /**< The size of the CID register in bytes */
#define HW_SDHC_CSD_SIZE                                (16UL)            /**< The size of the CSD register in bytes */
#define HW_SDHC_EXT_CSD_SIZE                            (512UL)           /**< The size of the EXT_CSD register in bytes */

/*
 *****************************************************************************************
 *
 * Low-level register functions.
 *
 *****************************************************************************************
 */

/**
 * \brief Register CLK_CTRL_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_clk_ctrl_r_internal_clk_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_CLK_CTRL_R_REG, INTERNAL_CLK_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_clk_ctrl_r_internal_clk_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CLK_CTRL_R_REG, INTERNAL_CLK_EN);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_clk_ctrl_r_internal_clk_stable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CLK_CTRL_R_REG, INTERNAL_CLK_STABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_clk_ctrl_r_freq_sel(HW_SDHC_ID id, uint8_t val)
{
        HW_SDHC_REG_SETF(id, EMMC_CLK_CTRL_R_REG, FREQ_SEL, val);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_clk_ctrl_r_freq_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CLK_CTRL_R_REG, FREQ_SEL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_clk_ctrl_r_upper_freq_sel(HW_SDHC_ID id, uint8_t val)
{
        ASSERT_WARNING(val <= HW_SDHC_MAX_UPPER_FREQ_SEL);
        HW_SDHC_REG_SETF(id, EMMC_CLK_CTRL_R_REG, UPPER_FREQ_SEL, val);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_clk_ctrl_r_upper_freq_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CLK_CTRL_R_REG, UPPER_FREQ_SEL);
}
#if (HW_SDHC_HC_CLOCK_GENERATOR_SUPPORTED == 1)
__STATIC_FORCEINLINE void hw_sdhc_set_clk_ctrl_r_clk_gen_select(HW_SDHC_ID id, HW_SDHC_CLK_CTRL_R_CLK_GEN_SELECT val)
{
        HW_SDHC_REG_SETF(id, EMMC_CLK_CTRL_R_REG, CLK_GEN_SELECT, val);
}
__STATIC_FORCEINLINE HW_SDHC_CLK_CTRL_R_CLK_GEN_SELECT hw_sdhc_get_clk_ctrl_r_clk_gen_select(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CLK_CTRL_R_REG, CLK_GEN_SELECT);
}
#endif
__STATIC_FORCEINLINE void hw_sdhc_set_clk_ctrl_r_pll_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_CLK_CTRL_R_REG, PLL_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_clk_ctrl_r_pll_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CLK_CTRL_R_REG, PLL_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_clk_ctrl_r_sd_clk_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_CLK_CTRL_R_REG, SD_CLK_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_clk_ctrl_r_sd_clk_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CLK_CTRL_R_REG, SD_CLK_EN);
}

/**
 * \brief Register PWR_CTRL_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_pwr_ctrl_r_sd_bus_vol_vdd1(HW_SDHC_ID id, uint8_t val)
{
        HW_SDHC_REG_SETF(id, EMMC_PWR_CTRL_R_REG, SD_BUS_VOL_VDD1, val);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_pwr_ctrl_r_sd_bus_vol_vdd1(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PWR_CTRL_R_REG, SD_BUS_VOL_VDD1);
}
__STATIC_FORCEINLINE void hw_sdhc_set_pwr_ctrl_r_sd_bus_vol_vdd2(HW_SDHC_ID id, uint8_t val)
{
        // Only for UHS-II
        HW_SDHC_REG_SETF(id, EMMC_PWR_CTRL_R_REG, SD_BUS_VOL_VDD2, val);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_pwr_ctrl_r_sd_bus_vol_vdd2(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PWR_CTRL_R_REG, SD_BUS_VOL_VDD2);
}
__STATIC_FORCEINLINE void hw_sdhc_set_pwr_ctrl_r_sd_bus_pwr_vdd1(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_PWR_CTRL_R_REG, SD_BUS_PWR_VDD1, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pwr_ctrl_r_sd_bus_pwr_vdd1(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PWR_CTRL_R_REG, SD_BUS_PWR_VDD1);
}

/**
 * \brief Register PSTATE
 */
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_pstate_dat_3_0(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, DAT_3_0);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_pstate_dat_7_4(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, DAT_7_4);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_cmd_inhibit(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, CMD_INHIBIT);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_cmd_inhibit_dat(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, CMD_INHIBIT_DAT);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_dat_line_active(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, DAT_LINE_ACTIVE);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_card_stable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, CARD_STABLE);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_card_inserted(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, CARD_INSERTED);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_buf_rd_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, BUF_RD_ENABLE);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_buf_wr_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, BUF_WR_ENABLE);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_buf_rd_xfer_active(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, RD_XFER_ACTIVE);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_buf_wr_xfer_active(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, WR_XFER_ACTIVE);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_pstate_cmd_issue_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_PSTATE_REG, CMD_ISSUE_ERR);
}


/**
 * \brief Register BLOCKSIZE_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_blocksize_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_BLOCKSIZE_R_REG = val;
}
__STATIC_FORCEINLINE void hw_sdhc_set_blocksize_r_sdma_buf_bdary(HW_SDHC_ID id, HW_SDHC_BLOCKSIZE_R_SDMA_BUF_BDARY val)
{
        HW_SDHC_REG_SETF(id, EMMC_BLOCKSIZE_R_REG, SDMA_BUF_BDARY, val);
}
__STATIC_FORCEINLINE HW_SDHC_BLOCKSIZE_R_SDMA_BUF_BDARY hw_sdhc_get_blocksize_r_sdma_buf_bdary(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_BLOCKSIZE_R_REG, SDMA_BUF_BDARY);
}
__STATIC_FORCEINLINE void hw_sdhc_set_blocksize_r_xfer_block_size(HW_SDHC_ID id, uint16_t val)
{
        ASSERT_WARNING((val > 0) && (val <= HW_SDHC_MAX_XFER_BLOCK_SIZE));
        HW_SDHC_REG_SETF(id, EMMC_BLOCKSIZE_R_REG, XFER_BLOCK_SIZE, val);
}
__STATIC_FORCEINLINE uint16_t hw_sdhc_get_blocksize_r_xfer_block_size(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_BLOCKSIZE_R_REG, XFER_BLOCK_SIZE);
}

/**
 * \brief Register BLOCKCOUNT_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_blockcount_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_BLOCKCOUNT_R_REG = val;
}
__STATIC_FORCEINLINE uint16_t hw_sdhc_get_blockcount_r(HW_SDHC_ID id)
{
        return (id)->EMMC_BLOCKCOUNT_R_REG;
}

/**
 * \brief Register ARGUMENT_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_argument_r(HW_SDHC_ID id, uint32_t val)
{
        (id)->EMMC_ARGUMENT_R_REG = val;
}

/**
 * \brief Register CMD_R
 *
 * Writing on the upper byte of Command register (i.e. CMD_INDEX) shall trigger
 * issuance of the an SD command.
 *
 */
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_CMD_R_REG = val;
}
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r_resp_type_select(HW_SDHC_ID id, HW_SDHC_CMD_R_RESP_TYPE_SELECT val)
{
        HW_SDHC_REG_SETF(id, EMMC_CMD_R_REG, RESP_TYPE_SELECT, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r_sub_cmd_flag(HW_SDHC_ID id, HW_SDHC_CMD_R_SUB_CMD_FLAG val)
{
        HW_SDHC_REG_SETF(id, EMMC_CMD_R_REG, SUB_CMD_FLAG, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r_cmd_crc_chk_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_CMD_R_REG, CMD_CRC_CHK_ENABLE, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r_cmd_idx_chk_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_CMD_R_REG, CMD_IDX_CHK_ENABLE, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r_data_present_sel(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_CMD_R_REG, DATA_PRESENT_SEL, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r_cmd_type(HW_SDHC_ID id, HW_SDHC_CMD_R_CMD_TYPE val)
{
        HW_SDHC_REG_SETF(id, EMMC_CMD_R_REG, CMD_TYPE, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_cmd_r_cmd_index(HW_SDHC_ID id, HW_SDHC_CMD_R_CMD_INDEX val)
{
        ASSERT_WARNING(val < HW_SDHC_CMD_INDEX_MAX_LIMIT);
        HW_SDHC_REG_SETF(id, EMMC_CMD_R_REG, CMD_INDEX, val);
}

/**
 * \brief Register HOST_CTRL1_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl1_r_dat_xfer_width(HW_SDHC_ID id, HW_SDHC_HOST_CTRL1_R_DAT_XFER_WIDTH val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL1_R_REG, DAT_XFER_WIDTH, val);
}
__STATIC_FORCEINLINE HW_SDHC_HOST_CTRL1_R_DAT_XFER_WIDTH hw_sdhc_get_host_ctrl1_r_dat_xfer_width(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL1_R_REG, DAT_XFER_WIDTH);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl1_r_card_detect_sig_sel(HW_SDHC_ID id, HW_SDHC_HOST_CTRL1_R_CARD_DETECT_SIG_SEL val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL1_R_REG, CARD_DETECT_SIG_SEL, val);
}
__STATIC_FORCEINLINE HW_SDHC_HOST_CTRL1_R_CARD_DETECT_SIG_SEL hw_sdhc_get_host_ctrl1_r_card_detect_sig_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL1_R_REG, CARD_DETECT_SIG_SEL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl1_r_card_detect_test_lvl(HW_SDHC_ID id, HW_SDHC_HOST_CTRL1_R_CARD_DETECT_TEST_LVL val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL1_R_REG, CARD_DETECT_TEST_LVL, val);
}
__STATIC_FORCEINLINE HW_SDHC_HOST_CTRL1_R_CARD_DETECT_TEST_LVL hw_sdhc_get_host_ctrl1_r_card_detect_test_lvl(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL1_R_REG, CARD_DETECT_TEST_LVL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl1_r_ext_dat_xfer(HW_SDHC_ID id, HW_SDHC_HOST_CTRL1_R_EXT_DAT_XFER val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL1_R_REG, EXT_DAT_XFER, val);
}
__STATIC_FORCEINLINE HW_SDHC_HOST_CTRL1_R_EXT_DAT_XFER hw_sdhc_get_host_ctrl1_r_ext_dat_xfer(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL1_R_REG, EXT_DAT_XFER);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl1_r_dma_sel(HW_SDHC_ID id, HW_SDHC_HOST_CTRL1_R_DMA_SEL val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL1_R_REG, DMA_SEL, val);
}
__STATIC_FORCEINLINE HW_SDHC_HOST_CTRL1_R_DMA_SEL hw_sdhc_get_host_ctrl1_r_dma_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL1_R_REG, DMA_SEL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl1_r_high_speed_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL1_R_REG, HIGH_SPEED_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl1_r_high_speed_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL1_R_REG, HIGH_SPEED_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl1_r_led_ctrl(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL1_R_REG, LED_CTRL, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl1_r_led_ctrl(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL1_R_REG, LED_CTRL);
}

/**
 * \brief Register XFER_MODE_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_XFER_MODE_R_REG = val;
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_resp_int_disable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, RESP_INT_DISABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_xfer_mode_r_resp_int_disable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, RESP_INT_DISABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_resp_err_chk_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, RESP_ERR_CHK_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_xfer_mode_r_resp_err_chk_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, RESP_ERR_CHK_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_resp_type(HW_SDHC_ID id, HW_SDHC_XFER_MODE_R_RESP_TYPE val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, RESP_TYPE, val);
}
__STATIC_FORCEINLINE HW_SDHC_XFER_MODE_R_RESP_TYPE hw_sdhc_get_xfer_mode_r_resp_type(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, RESP_TYPE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_multi_blk_sel(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, MULTI_BLK_SEL, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_xfer_mode_r_multi_blk_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, MULTI_BLK_SEL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_data_xfer_dir(HW_SDHC_ID id, HW_SDHC_XFER_MODE_R_DATA_XFER_DIR val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, DATA_XFER_DIR, val);
}
__STATIC_FORCEINLINE HW_SDHC_XFER_MODE_R_DATA_XFER_DIR hw_sdhc_get_xfer_mode_r_data_xfer_dir(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, DATA_XFER_DIR);
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_auto_cmd_enable(HW_SDHC_ID id, HW_SDHC_XFER_MODE_R_AUTO_CMD_ENABLE val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, AUTO_CMD_ENABLE, val);
}
__STATIC_FORCEINLINE HW_SDHC_XFER_MODE_R_AUTO_CMD_ENABLE hw_sdhc_get_xfer_mode_r_auto_cmd_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, AUTO_CMD_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_block_count_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, BLOCK_COUNT_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_xfer_mode_r_block_count_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, BLOCK_COUNT_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_xfer_mode_r_dma_en_emmc(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_XFER_MODE_R_REG, DMA_EN_EMMC, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_xfer_mode_r_dma_en_emmc(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_XFER_MODE_R_REG, DMA_EN_EMMC);
}

/**
 * \brief Register CAPABILITIES1_R
 */
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_capabilities1_r_base_clk_freq(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, BASE_CLK_FREQ);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_volt_18(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, VOLT_18);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_volt_30(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, VOLT_30);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_volt_33(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, VOLT_33);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_sys_addr_64_v4(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, SYS_ADDR_64_V4);
}
__STATIC_FORCEINLINE HW_SDHC_CAPABILITIES1_R_TOUT_CLK_UNIT hw_sdhc_get_capabilities1_r_tout_clk_unit(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, TOUT_CLK_UNIT);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_capabilities1_r_tout_clk_freq(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, TOUT_CLK_FREQ);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_async_int_support(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, ASYNC_INT_SUPPORT);
}
__STATIC_FORCEINLINE HW_SDHC_CAPABILITIES1_R_MAX_BLK_LEN hw_sdhc_get_capabilities1_r_max_blk_len(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, MAX_BLK_LEN);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_high_speed_support(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, HIGH_SPEED_SUPPORT);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_sdma_support(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, SDMA_SUPPORT);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities1_r_adma2_support(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES1_R_REG, ADMA2_SUPPORT);
}

/**
 * \brief Register CAPABILITIES2_R
 */
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_capabilities2_r_clk_mul(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES2_R_REG, CLK_MUL);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities2_r_uhs2_support(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES2_R_REG, UHS2_SUPPORT);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_capabilities2_r_vdd2_18v_support(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_CAPABILITIES2_R_REG, VDD2_18V_SUPPORT);
}

/**
 * \brief Register SW_RST_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_sw_rst_r(HW_SDHC_ID id, uint8_t val)
{
        (id)->EMMC_SW_RST_R_REG = val;
}
__STATIC_FORCEINLINE void hw_sdhc_set_sw_rst_r_sw_rst_all(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_SW_RST_R_REG, SW_RST_ALL, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_sw_rst_r_sw_rst_all(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_SW_RST_R_REG, SW_RST_ALL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_sw_rst_r_sw_rst_cmd(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_SW_RST_R_REG, SW_RST_CMD, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_sw_rst_r_sw_rst_cmd(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_SW_RST_R_REG, SW_RST_CMD);
}
__STATIC_FORCEINLINE void hw_sdhc_set_sw_rst_r_sw_rst_dat(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_SW_RST_R_REG, SW_RST_DAT, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_sw_rst_r_sw_rst_dat(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_SW_RST_R_REG, SW_RST_DAT);
}

/**
 * \brief Register BGAP_CTRL_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_bgap_ctrl_r_stop_bg_req(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_BGAP_CTRL_R_REG, STOP_BG_REQ, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_bgap_ctrl_r_stop_bg_req(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_BGAP_CTRL_R_REG, STOP_BG_REQ);
}
__STATIC_FORCEINLINE void hw_sdhc_set_bgap_ctrl_r_rd_wait_ctrl(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_BGAP_CTRL_R_REG, RD_WAIT_CTRL, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_bgap_ctrl_r_rd_wait_ctrl(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_BGAP_CTRL_R_REG, RD_WAIT_CTRL);
}

/*
 * Register NORMAL_INT_STAT_R
 *
 */
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_NORMAL_INT_STAT_R_REG = val;
}
__STATIC_FORCEINLINE uint16_t hw_sdhc_get_normal_int_stat(HW_SDHC_ID id)
{
        return (id)->EMMC_NORMAL_INT_STAT_R_REG;
}
__STATIC_FORCEINLINE void hw_sdhc_clr_normal_int_stat(HW_SDHC_ID id)
{
        uint16_t val = hw_sdhc_get_normal_int_stat(id);
        if (val) {
                hw_sdhc_set_normal_int_stat(id, val);
        }
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_r_err_interrupt(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_R_REG, ERR_INTERRUPT, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_err_interrupt(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, ERR_INTERRUPT);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_r_cmd_complete(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_R_REG, CMD_COMPLETE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_cmd_complete(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, CMD_COMPLETE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_r_xfer_complete(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_R_REG, XFER_COMPLETE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_xfer_complete(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, XFER_COMPLETE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_r_buf_wr_ready(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_R_REG, BUF_WR_READY, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_buf_wr_ready(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, BUF_WR_READY);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_r_buf_rd_ready(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_R_REG, BUF_RD_READY, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_buf_rd_ready(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, BUF_RD_READY);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_r_dma_interrupt(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_R_REG, DMA_INTERRUPT, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_dma_interrupt(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, DMA_INTERRUPT);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_card_interrupt_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, CARD_INTERRUPT);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_bgap_event(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_R_REG, BGAP_EVENT);
}


/**
 * \brief Register ERROR_INT_STAT_R
 *
 */
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_ERROR_INT_STAT_R_REG = val;
}
__STATIC_FORCEINLINE uint16_t hw_sdhc_get_error_int_stat_r(HW_SDHC_ID id)
{
        return (id)->EMMC_ERROR_INT_STAT_R_REG;
}
__STATIC_FORCEINLINE void hw_sdhc_clr_error_int_stat(HW_SDHC_ID id)
{
        uint16_t val = hw_sdhc_get_error_int_stat_r(id);
        if (val) {
                hw_sdhc_set_error_int_stat_r(id, val);
        }
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_r_adma_err(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_STAT_R_REG, ADMA_ERR, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_r_adma_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_R_REG, ADMA_ERR);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_r_cmd_idx_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_R_REG, CMD_IDX_ERR);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_r_cmd_end_bit_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_R_REG, CMD_END_BIT_ERR);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_r_data_crc_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_R_REG, DATA_CRC_ERR);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_r_data_tout_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_R_REG, DATA_TOUT_ERR);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_r_cmd_crc_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_R_REG, CMD_CRC_ERR);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_r_cmd_tout_err(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_R_REG, CMD_TOUT_ERR);
}

/**
 * \brief Register ERROR_INT_STAT_EN_R
 *
 */
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_en_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_ERROR_INT_STAT_EN_R_REG = val;
}
__STATIC_FORCEINLINE uint16_t hw_sdhc_get_error_int_stat_en_r(HW_SDHC_ID id)
{
        return (id)->EMMC_ERROR_INT_STAT_EN_R_REG;
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_en_r_adma_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, ADMA_ERR_STAT_EN, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_en_r_cmd_idx_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_IDX_ERR_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_en_r_cmd_idx_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_IDX_ERR_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_en_r_cmd_end_bit_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_END_BIT_ERR_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_en_r_cmd_end_bit_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_END_BIT_ERR_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_en_r_cmd_crc_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_CRC_ERR_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_en_r_cmd_crc_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_CRC_ERR_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_stat_en_r_cmd_tout_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_TOUT_ERR_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_stat_en_r_cmd_tout_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_STAT_EN_R_REG, CMD_TOUT_ERR_STAT_EN);
}

/**
 * \brief Register ERROR_INT_SIGNAL_EN_R
 *
 */
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_signal_en_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_ERROR_INT_SIGNAL_EN_R_REG = val;
}
__STATIC_FORCEINLINE uint16_t hw_sdhc_get_error_int_signal_en_r(HW_SDHC_ID id)
{
        return (id)->EMMC_ERROR_INT_SIGNAL_EN_R_REG;
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_signal_en_r_cmd_idx_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_IDX_ERR_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_signal_en_r_cmd_idx_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_IDX_ERR_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_signal_en_r_cmd_end_bit_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_END_BIT_ERR_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_signal_en_r_cmd_end_bit_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_END_BIT_ERR_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_signal_en_r_cmd_crc_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_CRC_ERR_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_signal_en_r_cmd_crc_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_CRC_ERR_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_signal_en_r_cmd_tout_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_TOUT_ERR_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_signal_en_r_cmd_tout_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, CMD_TOUT_ERR_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_error_int_signal_en_r_adma_err_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, ADMA_ERR_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_error_int_signal_en_r_adma_err_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_ERROR_INT_SIGNAL_EN_R_REG, ADMA_ERR_SIGNAL_EN);
}

/**
 * \brief Register ADMA_ERR_STAT_R
 *
 */
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_adma_err_stat_r(HW_SDHC_ID id)
{
        return (id)->EMMC_ADMA_ERR_STAT_R_REG;
}

/**
 * \brief Register SDMASA_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_sdmasa_r(HW_SDHC_ID id, uint32_t val)
{
        (id)->EMMC_SDMASA_R_REG = val;
}
__STATIC_FORCEINLINE uint32_t hw_sdhc_get_sdmasa_r(HW_SDHC_ID id)
{
        return (id)->EMMC_SDMASA_R_REG;
}

/**
 * \brief Register ADMA_SA_LOW_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_adma_sa_low_r(HW_SDHC_ID id, uint32_t val)
{
        (id)->EMMC_ADMA_SA_LOW_R_REG = val;
}
__STATIC_FORCEINLINE uint32_t hw_sdhc_get_adma_sa_low_r(HW_SDHC_ID id)
{
        return (id)->EMMC_ADMA_SA_LOW_R_REG;
}

/**
 * \brief Register BUF_DAT_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_buf_dat_r(HW_SDHC_ID id, uint32_t val)
{
        (id)->EMMC_BUF_DATA_R_REG = val;
}
__STATIC_FORCEINLINE uint32_t hw_sdhc_get_buf_dat_r(HW_SDHC_ID id)
{
        return (id)->EMMC_BUF_DATA_R_REG;
}

/**
 * \brief Register RESP01_R
 */
__STATIC_FORCEINLINE uint32_t hw_sdhc_get_resp01_r(HW_SDHC_ID id)
{
        return (id)->EMMC_RESP01_R_REG;
}

/**
 * Register RESP23_R
 */
__STATIC_FORCEINLINE uint32_t hw_sdhc_get_resp23_r(HW_SDHC_ID id)
{
        return (id)->EMMC_RESP23_R_REG;
}

/**
 * \brief Register RESP45_R
 */
__STATIC_FORCEINLINE uint32_t hw_sdhc_get_resp45_r(HW_SDHC_ID id)
{
        return (id)->EMMC_RESP45_R_REG;
}

/**
 * \brief Register RESP67_R
 */
__STATIC_FORCEINLINE uint32_t hw_sdhc_get_resp67_r(HW_SDHC_ID id)
{
        return (id)->EMMC_RESP67_R_REG;
}

/**
 * \brief Register NORMAL_INT_SIGNAL_EN_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_NORMAL_INT_SIGNAL_EN_R_REG = val;
}
__STATIC_FORCEINLINE uint16_t hw_sdhc_get_normal_int_signal_en_r(HW_SDHC_ID id)
{
        return (id)->EMMC_NORMAL_INT_SIGNAL_EN_R_REG;
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_card_interrupt_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CARD_INTERRUPT_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_signal_en_r_card_interrupt_signal_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CARD_INTERRUPT_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_card_insertion_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CARD_INSERTION_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_signal_en_r_card_insertion_signal_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CARD_INSERTION_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_card_removal_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CARD_REMOVAL_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_signal_en_r_card_removal_signal_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CARD_REMOVAL_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_cmd_complete_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CMD_COMPLETE_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_signal_en_r_cmd_complete_signal_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, CMD_COMPLETE_SIGNAL_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_xfer_complete_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, XFER_COMPLETE_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_buf_wr_ready_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, BUF_WR_READY_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_buf_rd_ready_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, BUF_RD_READY_SIGNAL_EN, val);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_signal_en_r_dma_interrupt_signal_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_SIGNAL_EN_R_REG, DMA_INTERRUPT_SIGNAL_EN, val);
}

/**
 * \brief Register NORMAL_INT_STAT_EN_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r(HW_SDHC_ID id, uint16_t val)
{
        (id)->EMMC_NORMAL_INT_STAT_EN_R_REG = val;
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_card_interrupt_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_INTERRUPT_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_en_r_card_interrupt_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_INTERRUPT_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_card_insertion_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_INSERTION_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_en_r_card_insertion_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_INSERTION_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_card_removal_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_REMOVAL_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_en_r_card_removal_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CARD_REMOVAL_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_cmd_complete_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CMD_COMPLETE_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_en_r_cmd_complete_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, CMD_COMPLETE_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_buf_rd_ready_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, BUF_RD_READY_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_buf_rd_ready_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, BUF_RD_READY_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_buf_wr_ready_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, BUF_WR_READY_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_buf_wr_ready_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, BUF_WR_READY_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_xfer_complete_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, XFER_COMPLETE_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_xfer_complete_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, XFER_COMPLETE_STAT_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_normal_int_stat_en_r_dma_interrupt_stat_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, DMA_INTERRUPT_STAT_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_normal_int_stat_r_dma_interrupt_stat_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_NORMAL_INT_STAT_EN_R_REG, DMA_INTERRUPT_STAT_EN);
}

/**
 * \brief Register TOUT_CTRL_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_tout_ctrl_r_tout_cnt(HW_SDHC_ID id, uint8_t val)
{
        ASSERT_WARNING(val <= HW_SDHC_TOUT_CNT_MAX_REG_FIELD_VAL);
        HW_SDHC_REG_SETF(id, EMMC_TOUT_CTRL_R_REG, TOUT_CNT, val);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_tout_ctrl_r_tout_cnt(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_TOUT_CTRL_R_REG, TOUT_CNT);
}

/**
 * \brief Register HOST_CNTRL_VERS_R
 */
__STATIC_FORCEINLINE HW_SDHC_HOST_CNTRL_VERS_R_SPEC_VERSION_NUM hw_sdhc_get_host_ctrl_vers_r_spec_version_num(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CNTRL_VERS_R_REG, SPEC_VERSION_NUM);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_host_ctrl_vers_r_vendor_version_num(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CNTRL_VERS_R_REG, VENDOR_VERSION_NUM);
}

/**
 * \brief Register HOST_CTRL2_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_signaling_en(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, SIGNALING_EN, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_signaling_en(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, SIGNALING_EN);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_sample_clk_sel(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, SAMPLE_CLK_SEL, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_sample_clk_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, SAMPLE_CLK_SEL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_exec_tuning(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, EXEC_TUNING, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_exec_tuning(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, EXEC_TUNING);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_cmd23_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, CMD23_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_cmd23_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, CMD23_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_preset_val_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, PRESET_VAL_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_preset_val_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, PRESET_VAL_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_host_ver4_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, HOST_VER4_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_host_ver4_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, HOST_VER4_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_addressing(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, ADDRESSING, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_addressing(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, ADDRESSING);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_async_int_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, ASYNC_INT_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_async_int_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, ASYNC_INT_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_uhs2_if_enable(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, UHS2_IF_ENABLE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_host_ctrl2_r_uhs2_if_enable(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, UHS2_IF_ENABLE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_uhs_mode_sel(HW_SDHC_ID id, uint8_t val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, UHS_MODE_SEL, val);
}
__STATIC_FORCEINLINE uint8_t hw_sdhc_get_host_ctrl2_r_uhs_mode_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, UHS_MODE_SEL);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_adma2_len_mode(HW_SDHC_ID id, HW_SDHC_ADMA2_LEN_MODE val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, ADMA2_LEN_MODE, val);
}
__STATIC_FORCEINLINE HW_SDHC_ADMA2_LEN_MODE hw_sdhc_get_host_ctrl2_r_adma2_len_mode(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, ADMA2_LEN_MODE);
}
__STATIC_FORCEINLINE void hw_sdhc_set_host_ctrl2_r_drv_strength_sel(HW_SDHC_ID id, HW_SDHC_HOST_CTRL2_R_DRV_STRENGTH_SEL val)
{
        HW_SDHC_REG_SETF(id, EMMC_HOST_CTRL2_R_REG, DRV_STRENGTH_SEL, val);
}
__STATIC_FORCEINLINE HW_SDHC_HOST_CTRL2_R_DRV_STRENGTH_SEL hw_sdhc_get_host_ctrl2_r_drv_strength_sel(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_HOST_CTRL2_R_REG, DRV_STRENGTH_SEL);
}

/**
 * \brief Register EMMC_CTRL_R
 */
__STATIC_FORCEINLINE void hw_sdhc_set_emmc_ctrl_r_card_is_emmc(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_EMMC_CTRL_R_REG, CARD_IS_EMMC, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_emmc_ctrl_r_card_is_emmc(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_EMMC_CTRL_R_REG, CARD_IS_EMMC);
}
__STATIC_FORCEINLINE void hw_sdhc_set_emmc_ctrl_r_disable_data_crc_chk(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_EMMC_CTRL_R_REG, DISABLE_DATA_CRC_CHK, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_emmc_ctrl_r_disable_data_crc_chk(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_EMMC_CTRL_R_REG, DISABLE_DATA_CRC_CHK);
}
__STATIC_FORCEINLINE void hw_sdhc_set_emmc_ctrl_r_emmc_rst_n(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_EMMC_CTRL_R_REG, EMMC_RST_N, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_emmc_ctrl_r_emmc_rst_n(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_EMMC_CTRL_R_REG, EMMC_RST_N);
}
__STATIC_FORCEINLINE void hw_sdhc_set_emmc_ctrl_r_emmc_rst_n_oe(HW_SDHC_ID id, bool val)
{
        HW_SDHC_REG_SETF(id, EMMC_EMMC_CTRL_R_REG, EMMC_RST_N_OE, val);
}
__STATIC_FORCEINLINE bool hw_sdhc_get_emmc_ctrl_r_emmc_rst_n_oe(HW_SDHC_ID id)
{
        return HW_SDHC_REG_GETF(id, EMMC_EMMC_CTRL_R_REG, EMMC_RST_N_OE);
}

/*
 *****************************************************************************************
 *
 * API Functions.
 *
 *****************************************************************************************
 */
/**
 * \brief Check if data transfer is active
 *
 * \param [in] id               SDHC controller instance
 *
 * \return true if transfer is in progress
 */
bool hw_sdhc_is_busy(HW_SDHC_ID id);

/**
 * \brief Wait while the card is busy, DAT0 is low, until timeout
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout, in ms
 *                              timeout of zero implies 1ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_wait_while_card_is_busy(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Send Command
 *
 * Programming sequence for issuing CMD without Data Transfer
 *
 * Registers NORMAL_INT_SIGNAL_EN_R and NORMAL_INT_STAT_EN_R are set to enable CMD_COMPLETE event
 * and the registered driver context is set accordingly
 *
 * \param [in] id               SDHC controller instance
 * \param [in] cmd_config       Command Configuration structure
 * \param [out] response        Pointer to response from card
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note: response must be valid until the end of the transaction, i.e. even after the function returns.
 * If the caller allocated response in the stack, it might become invalid.
 *
 * \note response might be NULL, in case no response is expected from the command sent
 */
HW_SDHC_STATUS hw_sdhc_send_command(HW_SDHC_ID id, const hw_sdhc_cmd_config_t *cmd_config, uint32_t *response);

/**
 * \brief Wait for CMD_COMPLETE event after sending a command
 *
 * Handle error events as well
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \sa hw_sdhc_send_command
 */
HW_SDHC_STATUS hw_sdhc_wait_cmd_complete_event(const HW_SDHC_ID id);

/**
 * \brief Wait for a timeout while the CMD line is inhibited
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_wait_cmd_line_not_inhibited(HW_SDHC_ID id);

/**
 * \brief Wait for a timeout while either DAT line is active or Read transfer is active
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_wait_data_line_not_inhibited(HW_SDHC_ID id);

/**
 * \brief Interrupt handler
 *
 * \param [in] id               SDHC controller instance
 */
void hw_sdhc_interrupt_handler(HW_SDHC_ID id);

/**
 * \brief Initialize data transfer
 *
 * \param [in] id               SDHC controller instance
 * \param [in] config           Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_data_xfer_init(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Initialize transfer related registers:
 *   SDMASA_R
 *   BLOCKSIZE_R
 *   BLOCKCOUNT_R
 *   ADMA_SA_LOW_R
 *
 * Note: SDMASA_R, BLOCKSIZE_R and BLOCKCOUNT_R are not stable and change their value
 * during data xfer. Therefore the Host Driver should not read them.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] config           Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_xfer_registers(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Send command to issue data transfer
 *
 * Wait for response R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] config           Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_data_xfer_send_cmd(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Start non-DMA and blocking data transfer
 *
 * Blocking: wait for data transfer events without interrupt handling
 *
 * \param [in] id               SDHC controller instance
 * \param [inout] config        Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Parameter config should be valid until the transaction is complete,
 *       since it contains the data read or written.
 */
HW_SDHC_STATUS hw_sdhc_data_xfer_start_non_dma_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Start non-DMA and non-blocking data transfer
 *
 * Non-Blocking: the interrupt handler is called when data transfer is complete
 *
 * \param [in] id               SDHC controller instance
 * \param [inout] config        Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Parameter config should be valid until the transaction is complete,
 *       since it contains the data read or written.
 */
HW_SDHC_STATUS hw_sdhc_data_xfer_start_non_dma_non_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Start DMA and blocking data transfer
 *
 * Blocking: wait for data transfer events without interrupt handling
 *
 * \note: In cases of error, error recovery function and/or abort data transfer are called
 *
 * \param [in] id               SDHC controller instance
 * \param [inout] config        Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Parameter config should be valid until the transaction is complete,
 *       since it contains the data read or written.
 */
HW_SDHC_STATUS hw_sdhc_data_xfer_start_dma_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Start DMA and non-blocking data transfer
 *
 * Non-Blocking: the interrupt handler is called when data transfer is complete
 *
 * \note: In cases of error, error recovery function is called
 *
 * \param [in] id               SDHC controller instance
 * \param [inout] config        Data transfer configuration structure
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Parameter config should be valid until the transaction is complete,
 *       since it contains the data read or written.
 */
HW_SDHC_STATUS hw_sdhc_data_xfer_start_dma_non_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);

/**
 * \brief Wait for a timeout while data transfer is not complete
 *
 * \note This timeout is fixed and defined the same for both read and write transfers
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout             timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \sa HW_SDHC_XFER_COMPLETE_TOUT_MS
 */
HW_SDHC_STATUS hw_sdhc_wait_xfer_complete_event(HW_SDHC_ID id, uint32_t tout);

/**
 * \brief Wait for a timeout while data read buffer is not ready
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_wait_buf_rd_ready(HW_SDHC_ID id);

/**
 * \brief Wait for a timeout while data read buffer is not enabled
 * Wait for valid data in the data buffer
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_wait_buf_rd_enable(HW_SDHC_ID id);

/**
 * \brief Wait for a timeout while data write buffer is not ready
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_wait_buf_wr_ready(HW_SDHC_ID id);

/**
 * \brief Wait for a timeout while data write buffer is not enabled
 * Wait for valid data in the data buffer
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_wait_buf_wr_enable(HW_SDHC_ID id);

/**
 * \brief Wait for a timeout while data transfer is not complete
 *
 * \note This timeout is fixed and defined the same for both read and write transfers
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout             timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \sa HW_SDHC_XFER_COMPLETE_TOUT_MS
 */
HW_SDHC_STATUS hw_sdhc_wait_xfer_complete(HW_SDHC_ID id, uint32_t tout);

/**
 * \brief Command or Data transfer (read/write) event handling is complete
 *
 * \param [in] id               SDHC controller instance
 * \param [in] events           events occurred at transaction
 */
void hw_sdhc_evt_complete(HW_SDHC_ID id, uint32_t events);

/**
 * \brief Abort data transfer synchronously
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_abort_xfer_sync(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Abort data transfer asynchronously
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_abort_xfer_async(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief It is called after error interrupts are triggered
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Instead of HW_SDHC_STATUS_RECOVERABLE_ERROR, return HW_SDHC_STATUS_SUCCESS
 *       to be consistent with other API functions return values
 */
HW_SDHC_STATUS hw_sdhc_error_recovery(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Assert bus speed
 *
 * Bus speed should be less than the max speed the IP is configured and
 * greater than the card identification frequency
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_speed        Bus speed in Hz
 *
 * \return true if OK
 */
bool hw_sdhc_assert_bus_speed(HW_SDHC_ID id, uint32_t bus_speed);

/**
 * \brief Assert clock divider has a valid value
 *
 * \param [in] id               SDHC controller instance
 * \param [in] clk_div          Clock divider
 *
 * \return true if OK
 */
bool hw_sdhc_assert_clk_div(HW_SDHC_ID id, uint8_t clk_div);

/**
 * \brief Assert bus width and bus speed mode
 *
 * DDR mode is valid only when bus width is more than one
 *
 * \note: DDR mode is not currently supported
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_width        Bus width
 * \param [in] speed_mode       Speed mode
 *
 * \return true if OK
 */
bool hw_sdhc_assert_bus_width_and_speed_mode(const HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width, uint8_t speed_mode);

/**
 * \brief Assert bus speed and bus speed mode
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_speed        Bus speed in Hz
 * \param [in] speed_mode       Speed mode
 *
 * \return true if OK
 */
bool hw_sdhc_assert_bus_speed_and_speed_mode(const HW_SDHC_ID id, uint32_t bus_speed, uint8_t speed_mode);

/**
 * \brief eMMC driver registers its context to the SD HC driver
 *
 * \note: This function SHOULD be called when eMMC driver is in FREE state
 *
 * \param [in] id               SDHC controller instance
 * \param [in] context          Pointer to the eMMC context data
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 *\note The parameter context must stay valid until \sa hw_sdhc_unregister_context() is called
 */
HW_SDHC_STATUS hw_sdhc_register_context(const HW_SDHC_ID id, hw_sdhc_context_data_t *context);

/**
 * \brief eMMC driver unregisters its context to the SD HC driver
 *
 * \param [in] id       SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_unregister_context(const HW_SDHC_ID id);

/**
 * \brief Set normal and error interrupts mask
 *
 * \param [in] id               SDHC controller instance
 * \param [in] normal_int_mask  Normal interrupts mask
 * \param [in] error_int_mask   Error interrupts mask
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_active_interrupts_mask(HW_SDHC_ID id, uint16_t normal_int_mask, uint16_t error_int_mask);


/**
 * \brief Software reset for DAT line
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_and_wait_sw_rst_dat(HW_SDHC_ID id);

/**
 * \brief Software reset for CMD line
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_and_wait_sw_rst_cmd(HW_SDHC_ID id);

/**
 * \brief Wait for power ramp-up
 *
 * Ramp-up time = MAX {1ms, 74 clk cycles, Card supply ramp-up time, boot operation period} = 1ms
 *   Max 74 clk cycles delay = 74 x (1/125kHz) = 592 usec
 *   Card supply ramp-up time = 0, since devices are embedded
 *   Boot operation period = 0, since boot is not implemented yet
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_speed        Bus speed in Hz
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
void hw_sdhc_wait_power_ramp_up(HW_SDHC_ID id, uint32_t bus_speed);

/**
 * \brief SD Stop Clock Sequence
 *
 * \note Wait while an SD transaction is executing in the SD bus before stopping the SD clock
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_stop_sd_clock(HW_SDHC_ID id);

/**
 * Timeout Setting
 * There are 14 discrete (coarse) values for TOUT_CNT.
 *
 * - If unit is in MHz then tout should be in usec.
 *
 * - If unit is in kHz then tout should be in msec.
 *      If tout was in usec, when unit is kHz then:
 *              tout = tout_cnt x (1000 / F_KHZ)
 *              =>
 *              tout(min) = 2^13 x (1000 / 32) = 2^8 x 1000 = 256 ms
 *              tout(max) = 2^27 x (1000 / 32) = 2^22 x 1000 = 4.194.304 ms
 *
 * tout = tout_cnt x (1/F)
 * =>
 * tout_cnt = tout x F
 *
 * If F = tout_clk_freq = 32 MHz = 2^5 MHZ
 * =>
 * tout(min) = 2^13 x (1/32) = 2^8 MHZ = 256 us
 * tout(max) = 2^27 x (1/32) = 2^22 MHZ= 4.194.304 us ~ 4,2 sec
 *
 * Pseudo-code:
 *      tout_cnt = (8*sizeof(uint32_t) - __CLZ(tout x F_MHZ)) - 1;
 *      if (2^tout_cnt < tout x F_MHZ) then tout_cnt++;
 *      TOUT_CNT = tout_cnt - 13;
 *      Actual tout = 2^tout_cnt/F
 *
 * Examples:
 * 1. tout=256 usec => tout_cnt=(32 - __CLZ(256x32))=13. But 2^13==256x32=> TOUT_CNT=13-13=0
 *    Actual tout=2^13/32=256 usec
 *
 * 2. tout=300 usec => tout_cnt=(32 - __CLZ(300x32))=13. But 2^13<300x32=> TOUT_CNT=14-13=1
 *    Actual tout=2^14/32=512 usec
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout             Timeout value
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 */
HW_SDHC_STATUS hw_sdhc_timeout_setting(HW_SDHC_ID id, uint32_t tout);

/**
 * \brief Enable internal clock and wait for it to be stable
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_internal_clk_enable(HW_SDHC_ID id);

/**
 * \brief Set Host Controller and bus speed frequency (Hz), SD clock is enabled
 *
 * \note If the frequency has been already set, then return
 *
 * \param [in] id               SDHC controller instance
 * \param [in] frequency        HC and bus speed frequency in Hz
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_frequency(HW_SDHC_ID id, uint32_t frequency);

/**
 * \brief Set bus width at Host Controller
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_width        Bus width
 */
void hw_sdhc_set_bus_width_at_host(HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width);

/*
 * Commands
 */

/**
 * \brief GO_IDLE_STATE, reset card
 *
 * CMD0 is a special command. It is required for the card initialization.
 * However, CMD_COMPLETE is not received so do not wait for it.
 * A delay is added after CMD0 which is required before CMD1.
 *
 * \note: it is an ABORT command type.
 *
 * \note: wait for No response
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_go_idle_state_CMD0(HW_SDHC_ID id);

/**
 * \brief SELECT/DESELECT_CARD, toggle a card between stand-by and transfer states or
 * between programming and disconnect states.
 *
 * When de-selecting the card, should not check the return value.
 *
 * Response: R1/R1b (wait for busy)
 *
 * R1 while selecting from Stand-By State to Transfer State
 * R1b (wait for busy) while selecting from Disconnected State to Programming State
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card address
 * \param [in] wait_for_busy    if true wait for busy
 * \param [in] busy_tout_ms     busy timeout in msec
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_select_deselect_card_CMD7(HW_SDHC_ID id, uint16_t rca, bool wait_for_busy, uint32_t busy_tout_ms);

#if (dg_configUSE_HW_EMMC == 1)
/**
 * \brief SEND_OP_COND, send eMMC operation condition command.
 * Asks Device, in idle state, to send its Operating Conditions Register contents
 *
 * Response: R3 (OCR)
 *
 * \param [in] id               SDHC controller instance
 * \param [out] ocr             Operating Conditions Register
 * \param [in] cmd_arg          Command Argument
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_send_op_cond_CMD1(HW_SDHC_ID id, uint32_t *ocr, uint32_t cmd_arg);

/**
 * \brief ALL_SEND_CID, asks any card to send the CID numbers on the CMD line
 * Card Identification Register (CID)
 *
 * Response: R2
 *
 * \param [in] id               SDHC controller instance
 * \param [inout] cid           Card Identification register
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_all_send_cid_CMD2(HW_SDHC_ID id, uint32_t *cid);

/**
 * \brief SET_RELATIVE_ADDR, assigns to the card a Relative Card Address (RCA)
 * The card goes from Identification to Stand-by state
 * Response: R1
 *
 * \note Relative card address 0x0000 is reserved
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Assigned Relative Card Address (RCA)
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_relative_address_CMD3(HW_SDHC_ID id, uint16_t rca);

/**
 * \brief SET_DSR, Programs the 16-bit Driver Stage Register (DSR) of the card
 * DSR is used to configure the card output drivers (bus). It is OPTIONAL.
 * The CSD register carries the information about the DSR register usage,
 * whether the card has implemented or not this function.
 * The default value of the DSR register is 0x404.
 *
 * Response: No response
 *
 * \param [in] id               SDHC controller instance
 * \param [in] dsr              DSR value
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_dsr_CMD4(HW_SDHC_ID id, uint16_t dsr);

/**
 * \brief Switch an eMMC card between a Sleep and a Standby state, using the command SLEEP_AWAKE (CMD5)
 *
 * \note If the card is not in Standby state, it cannot execute the sleep command
 *       If the card is in Sleep state, it reacts only to the commands RESET (CMD0) and AWAKE (CMD5)
 *
 * Response: R1b
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card Address
 * \param [in] sleep            Execute sleep command if true, otherwise execute awake command
 * \param [in] tout_ms          timeout for state transition, in ms.
 *                              The maximum value of tout_ms is defined in EXT_CSD[217] = S_A_TIMEOUT
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_emmc_sleep_awake_CMD5(HW_SDHC_ID id, uint32_t rca, bool sleep, uint32_t tout_ms);

/**
 * \brief SWITCH, switches the card operation mode or modifies the EXT_CSD register
 *
 * \note SWITCH command is effective only during transfer state.
 *
 * CMD6 Argument:
 * Cmd Set      [2:0]
 * Reserved     [7:3] = 0
 * Value        [15:8]
 * Index        [23:16]
 * Access       [25:24]
 * Reserved     [31:26] = 0
 *
 * \note Data_strobe is used for HS400 mode, not supported
 *
 * Response: R1b
 *
 * \param [in] id               SDHC controller instance
 * \param [in] config           configuration of SWITCH command
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_emmc_switch_CMD6(HW_SDHC_ID id, const hw_sdhc_switch_cmd6_config_t *config);

/**
 * \brief SWITCH, switches the card operation mode or modifies the EXT_CSD register
 *        Change Bus Speed mode for an eMMC Device.
 *
 * \note SWITCH command is effective only during transfer state.
 *
 * CMD6 Argument:
 * Cmd Set      [2:0] = 0
 * Reserved     [7:3] = 0
 * Value        [15:8] = speed mode
 * Index        [23:16] = 185 for HS Timing. Index of EXT_CSD table.
 * Access       [25:24] = 0 (Cmd Set), 1 (Set Bits), 2 (Clear bits), 3 (Write Byte)
 * Reserved     [31:26] = 0
 *
 * \note Data_strobe is used for HS400 mode, not supported
 *
 * Response: R1b
 *
 * \param [in] id               SDHC controller instance
 * \param [in] speed_mode       Speed mode
 * \param [in] hs_timing        high speed timing, based on CSD:SPEC_VER and speed mode
 * \param [in] tout_ms          Response busy timeout in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_emmc_speed_mode_CMD6(HW_SDHC_ID id, HW_SDHC_HOST_CTRL2_R_EMMC_BUS_SPEED_MODE_SEL speed_mode, uint8_t hs_timing, uint32_t tout_ms);

/**
 * \brief SWITCH, switches the card operation mode or modifies the EXT_CSD register
 *        Change Data Bus Width for an eMMC Device.
 *
 * \note SWITCH command is effective only during transfer state.
 *
 * CMD6 Argument:
 * Cmd Set      [2:0] = 0
 * Reserved     [7:3] = 0
 * Value        [15:8] = HW_SDHC_BUS_WIDTH_1_BIT=0, HW_SDHC_BUS_WIDTH_4_BIT=1, or HW_SDHC_BUS_WIDTH_8_BIT=2
 * Index        [23:16] = 183 for Bus Width. Index of EXT_CSD table.
 * Access       [25:24] = 0 (Cmd Set), 1 (Set Bits), 2 (Clear bits), 3 (Write Byte)
 * Reserved     [31:26] = 0
 *
 * Response: R1b
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_width        Bus width
 * \param [in] tout_ms          Response busy timeout in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_emmc_data_bus_width_CMD6(HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width, uint32_t tout_ms);

/**
 * \brief SEND_EXT_CSD
 *
 * \note The card sends its EXT_CSD register as a block of data
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card Address
 * \param [out] ext_csd         Extended CSD register
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_emmc_send_ext_csd_CMD8(HW_SDHC_ID id, uint16_t rca, uint8_t *ext_csd);

/**
 * \brief SEND_CSD, addressed card sends its Card Specific Data (CSD) on the CMD line
 *
 * Response: R2
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card Address
 * \param [out] csd             CSD register
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_send_csd_CMD9(HW_SDHC_ID id, uint16_t rca, uint32_t *csd);

/**
 * \brief SEND_CID, asks addressed card to send the CID numbers on the CMD line
 * Card Identification Register (CID)
 *
 * \note Should be called in Standby state
 *
 * Response: R2
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card Address
 * \param [out] cid             Card Identification register
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_send_cid_CMD10(HW_SDHC_ID id, uint16_t rca, uint32_t *cid);

/**
 * \brief Stop either an infinite or a multiple block transaction
 *
 * CMD12 Argument:
 * HPI          [0] = If set, the device shall interrupt its internal operation in a well defined manner
 * Stuff bits   [15:1] = 0
 * RCA          [31:16] = relative Card Address
 *
 * Response: R1/R1b
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card Address
 * \param [in] hpi              HPI flag
 * \param [in] tout_ms          card busy timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_stop_transmission_CMD12(HW_SDHC_ID id, uint16_t rca, bool hpi, uint32_t tout_ms);

/**
 * \brief SEND_STATUS: addressed card sends its status register
 *
 * CMD13 Argument:
 * HPI          [0] = If set, the device shall interrupt its internal operation in a well defined manner
 * Stuff bits   [15:1] = 0
 * RCA          [31:16] = relative Card Address
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card Address
 * \param [in] hpi              HPI flag
 * \param [out] card_status     Card status
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_send_status_CMD13(HW_SDHC_ID id, uint16_t rca, bool hpi, uint32_t *card_status);

/**
 * \brief GO_INACTVE_STATE: sets the addressed card to Inactive state
 * After the command GO_INACTVE_STATE, the device does not accept CMD0
 * The card will reset to Pre-idle state with power cycle
 *
 * Response: No response
 *
 * \param [in] id               SDHC controller instance
 * \param [in] rca              Relative Card Address
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_go_inactive_state_CMD15(HW_SDHC_ID id, uint16_t rca);

/**
 * \brief SET_BLOCKLEN: Sets the block length (in bytes) for all following
 * block commands (read and write). Default block length is specified in the CSD.
 *
 * \note At Dual Data Rate (DDR) operating mode, this command becomes illegal
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] blk_len          Block length
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_blocklen_CMD16(HW_SDHC_ID id, uint32_t blk_len);

/**
 * \brief SET_BLOCK_COUNT, specify block count for CMD18 and CMD25 (read/write multiple blocks)
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] reliable_wr      Reliable write
 * \param [in] blk_cnt          Block count
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_set_block_count_CMD23(HW_SDHC_ID id, bool reliable_wr, uint32_t blk_cnt);

/**
 * \brief PROGRAM_CID (CMD26)
 *
 * Programming of the card identification register. This command shall be issued at most once.
 * The card contains hardware to prevent this operation after the first programming.
 * Normally this command is reserved for the manufacturer.
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] buf              data buffer as long as the CID register (16 bytes)
 * \param [in] tout_ms          data transfer timeout in msec
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Returns success even if the registers are not programmed.
 * Therefore, send CMD13 to read card status and check bit HW_SDHC_CARD_STATUS_CID_CSD_OVERWRITE
 */
HW_SDHC_STATUS hw_sdhc_program_cid_CMD26(HW_SDHC_ID id, const uint8_t *buf, uint32_t tout_ms);

/**
 * \brief PROGRAM_CSD (CMD27)
 *
 * Programming of the programmable bits of the CSD.
 * The read-only part of the CSD should match the card content.
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] buf              data buffer as long as the CSD register (16 bytes)
 * \param [in] tout_ms          data transfer timeout in msec
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Returns success even if the registers are not programmed.
 * Therefore, send CMD13 to read card status and check bit HW_SDHC_CARD_STATUS_CID_CSD_OVERWRITE
 */
HW_SDHC_STATUS hw_sdhc_program_csd_CMD27(HW_SDHC_ID id, const uint8_t *buf, uint32_t tout_ms);

/**
 * \brief SET_WRITE_PROT (CMD28)
 *
 * If the card has write protection features, this command sets the write protection bit
 * of the addressed group. The properties of write protection are coded in the CSD:WP_GRP_SIZE or
 * EXT_CSD:HC_WP_GRP_SIZE.
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports write protection commands
 * if it is Class 6, i.e. bit 6 of CSD:CCC is set.
 *
 * Response: R1b (wait for busy)
 *
 * \param [in] id               SDHC controller instance
 * \param [in] data_addr        Data address
 * \param [in] tout_ms          timeout for state transition, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Data address for media =<2GB is a 32bit byte address and data address for media > 2GB
 * is a 32bit sector (512B) address.
 */
HW_SDHC_STATUS hw_sdhc_set_write_prot_CMD28(HW_SDHC_ID id, uint32_t data_addr, uint32_t tout_ms);

/**
 * \brief CLR_WRITE_PROT (CMD29)
 *
 * If the card provides write protection features, this command clears the write protection
 * bit of the addressed group.
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports write protection commands
 * if it is Class 6, i.e. bit 6 of CSD:CCC is set.
 *
 * Response: R1b (wait for busy)
 *
 * \param [in] id               SDHC controller instance
 * \param [in] data_addr        Data address
 * \param [in] tout_ms          timeout for state transition, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Data address for media =<2GB is a 32bit byte address and data address for media > 2GB
 * is a 32bit sector (512B) address.
 */
HW_SDHC_STATUS hw_sdhc_clr_write_prot_CMD29(HW_SDHC_ID id, uint32_t data_addr, uint32_t tout_ms);

/**
 * \brief SEND_WRITE_PROT (CMD30)
 *
 * If the card provides write protection features, this command asks the card to send the
 * status of the write protection bits.
 *
 * 32 write protection bits (representing 32 write protect groups starting at the specified address)
 * followed by 16 CRC bits are transferred in a payload format via the data lines.
 * The last (least significant) bit of the protection bits corresponds to the first addressed group.
 * If the addresses of the last groups are outside the valid range, then the corresponding write
 * protection bits shall be set to zero.
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports write protection commands
 * if it is Class 6, i.e. bit 6 of CSD:CCC is set.
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] wp_addr          write protect data address
 * \param [out] wp_status       status of the write protection bits
 * \param [in] tout_ms          data transfer timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Data address for media =<2GB is a 32bit byte address and data address for media > 2GB
 * is a 32bit sector (512B) address.
 */
HW_SDHC_STATUS hw_sdhc_send_write_prot_CMD30(HW_SDHC_ID id, uint32_t wp_addr, uint32_t *wp_status, uint32_t tout_ms);

/**
 * \brief SEND_WRITE_PROT_TYPE (CMD31)
 *
 * This command sends the type of write protection that is set for the different write protection
 * groups.
 *
 * 64 write protection bits (representing 32 write protect groups starting at the specified address)
 * followed by 16 CRC bits are transferred in a payload format via the data lines.
 * Each set of two protection bits shows the type of protection set for each of the write protection
 * groups. The definition of the different bit settings are shown below. The last (least significant)
 * two bits of the protection bits correspond to the first addressed group. If the addresses of the
 * last groups are outside the valid range, then the corresponding write protection bits shall be set
 * to zero.
 * "00" Write protection group is not protected
 * "01" Write protection group is protected by temporary write protection
 * "10" Write protection group is protected by power-on write protection
 * "11" Write protection group is protected by permanent write protection
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports write protection commands
 * if it is Class 6, i.e. bit 6 of CSD:CCC is set.
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] wp_addr          write protect data address
 * \param [out] wp_type         type of the write protection bits
 * \param [in] tout_ms          data transfer timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Data address for media =<2GB is a 32bit byte address and data address for media > 2GB
 * is a 32bit sector (512B) address.
 */
HW_SDHC_STATUS hw_sdhc_send_write_prot_type_CMD31(HW_SDHC_ID id, uint32_t wp_addr, uint64_t *wp_type, uint32_t tout_ms);

/**
 * \brief ERASE_GROUP_START (CMD35)
 *
 * Sets the address of the first erase group or block (trim) within a range to be selected for erase.
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] data_addr        start data address to be erased
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Data address for media =<2GB is a 32bit byte address and data address for media > 2GB
 * is a 32bit sector (512B) address.
 */
HW_SDHC_STATUS hw_sdhc_erase_group_start_CMD35(HW_SDHC_ID id, uint32_t data_addr);

/**
 * \brief ERASE_GROUP_END (CMD36)
 *
 * Sets the address of the last erase group or block (trim) within a continuous range to be selected for erase
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] data_addr        last data address to be erased
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note Data address for media =<2GB is a 32bit byte address and data address for media > 2GB
 * is a 32bit sector (512B) address.
 */
HW_SDHC_STATUS hw_sdhc_erase_group_end_CMD36(HW_SDHC_ID id, uint32_t data_addr);

/**
 * \brief ERASE (CMD38)
 *
 * Erases all previously selected write blocks according to argument bits
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports erase commands if it is Class 5,
 * i.e. bit 5 of CSD:CCC is set.
 *
 * Response: R1b (wait for busy)
 *
 * \param [in] id               SDHC controller instance
 * \param [in] arg              argument of CMD38 to select erase, trim, simple or secure
 * \param [in] tout_ms          timeout of CMD38, in ms
 *                              This value should be calculated for all groups to be erased
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 *
 * \note It is required to read EXT_CSD[231] = SEC_FEATURE_SUPPORT
 *
 * \note Data address for media =<2GB is a 32bit byte address and data address for media > 2GB
 * is a 32bit sector (512B) address.
 */
HW_SDHC_STATUS hw_sdhc_erase_CMD38(HW_SDHC_ID id, HW_SDHC_CMD38_ARG arg, uint32_t tout_ms);

/**
 * \brief LOCK_UNLOCK (CMD42)
 *
 * Sets/resets the password, locks/unlocks the card or forces a card erase.
 *
 * The card lock/unlock command (CMD42) has the structure and bus transaction type of a regular single
 * block write command.
 *
 * The data block structure of CMD42 is as follows:
 *   - Byte 0: card lock/unlock mode, i.e. ERASE, LOCK, UNLOCK, CLR_PWD, SET_PWD.
 *   - Byte 1: password length (PWD_LEN)
 *   - Byte 2 to N: password data
 * In case a single password is included in the data block, PWD_LEN = 1 to 16 and N = 2 to 17
 * In case of password replacement where both passwords (the old and the new one) are included
 * in the data block, PWD_LEN = 2 to 32 and N = 3 to 33.
 *
 * The size (N+1) of the data block is set by the SET_BLOCK_LEN (CMD16) command that should be called first.
 * The card should be selected (CMD7) before calling CMD16 and CMD42, i.e. should be in Transfer State.
 *
 * In case of ERASE, Byte 0 is only sent and the data block size is 1 (CMD16).
 * The ERASE operation can be executed only when the card is locked.
 *
 * A locked card cannot execute data transfer commands and in such case the CARD_IS_LOCKED (bit 25)
 * is set in the status register.
 *
 * An attempt to use password protection features (CMD42) on a card having password permanently disabled
 * will fail and the LOCK_UNLOCK_FAILED (bit 24) error bit will be set in the status register.
 * The password protection feature can be disabled permanently by setting the permanent password disable
 * bit (PERM_PSWD_DIS bit in the EXT_CSD byte [171]).
 *
 * The LOCK_UNLOCK_FAILED bit in the status register (bit 24) is set when a sequence or password error
 * has been detected in lock/unlock card command.
 *
 * CMD42 is an illegal command in Dual Data Rate (DDR) mode.
 *
 * The supported Card Command Classes (CCC) are coded in the CSD register of each card, providing the
 * host with information on how to access the card. A card supports lock/unlock commands if it is Class 7,
 * i.e. bit 7 of CSD:CCC is set.
 *
 * Response: R1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] len              data block length
 * \param [in] data             data block
 * \param [in] tout_ms          data transfer timeout, in ms
 *                              In case of force erase, tout_ms should be set at 3 minutes
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
HW_SDHC_STATUS hw_sdhc_lock_unlock_CMD42(HW_SDHC_ID id, uint8_t len, uint8_t *data, uint32_t tout_ms);

#endif


#endif /* (dg_configUSE_HW_EMMC == 1) || defined(__HW_SDHC_USE_HW_EMMC_ONLY) */

#endif /* HW_SDHC_H_ */

/**
 * \}
 * \}
 */
