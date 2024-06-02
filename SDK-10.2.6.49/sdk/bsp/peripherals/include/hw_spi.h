/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_SPI SPI Driver
 * \{
 * \brief Serial Peripheral Interface (SPI) Controller
 */

/**
 *
 *
 * @file hw_spi.h
 *
 * \brief Serial Peripheral Interface (SPI) Low Level Driver (LLD) API definition.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *
 */

#ifndef HW_SPI_H_
#define HW_SPI_H_


#if dg_configUSE_HW_SPI

#include <stdint.h>
#include "sdk_defs.h"
#include "hw_gpio.h"

/* SPI Base Address */
#define SBA(id)                         ((SPI_Type *)id)

typedef void (*hw_spi_tx_callback)(void *user_data, uint16_t transferred);

/**
 * \brief SPI controller number
 *
 */
#define HW_SPI1         ((void *)SPI_BASE)
#ifdef SPI2
#define HW_SPI2         ((void *)SPI2_BASE)
#endif
#ifdef SPI3
#define HW_SPI3         ((void *)SPI3_BASE)
#endif
typedef void * HW_SPI_ID;

/*
 * OPTIMIZATION DEFINITIONS
 **
 */

/**
 * \def HW_SPI_DMA_SUPPORT
 *
 * \brief DMA support for SPI
 *
 */
#define HW_SPI_DMA_SUPPORT              dg_configSPI_DMA_SUPPORT

/**
 * \brief Use predefined (fixed) word size to optimize performance.
 *
 * The SPI controller supports multiple word sizes (see \ref HW_SPI_WORD). However, if an
 * application uses devices with the same word size connected to the controller, a predefined word
 * size can be used by defining the HW_SPI1_FIXED_WORD_SIZE macro. This improves LLD performance.
 */
#ifndef HW_SPI1_USE_FIXED_WORD_SIZE
#define HW_SPI1_USE_FIXED_WORD_SIZE     (0)
#endif

#ifdef HW_SPI2
/**
 * \brief Use predefined (fixed) word size to optimize performance.
 *
 * The SPI2 controller supports multiple word sizes (see \ref HW_SPI_WORD). However, if an
 * application uses devices with the same word size connected to the controller, a predefined word
 * size can be used by defining the HW_SPI2_FIXED_WORD_SIZE macro. This improves LLD performance.
 */
#ifndef HW_SPI2_USE_FIXED_WORD_SIZE
#define HW_SPI2_USE_FIXED_WORD_SIZE     (0)
#endif
#endif

#ifdef HW_SPI3
/**
 * \brief Use predefined (fixed) word size to optimize performance.
 *
 * The SPI3 controller supports multiple word sizes (see \ref HW_SPI_WORD). However, if an
 * application uses devices with the same word size connected to the controller, a predefined word
 * size can be used by defining the HW_SPI3_FIXED_WORD_SIZE macro. This improves LLD performance.
 */
#ifndef HW_SPI3_USE_FIXED_WORD_SIZE
#define HW_SPI3_USE_FIXED_WORD_SIZE     (0)
#endif
#endif

#if HW_SPI1_USE_FIXED_WORD_SIZE == 1
/**
 * \brief SPI controller fixed word size value.
 *
 * This macro must be set by the user when \ref HW_SPI1_USE_FIXED_WORD_SIZE is set. See \ref
 * HW_SPI_WORD for the allowed values.
 */
#ifndef HW_SPI1_FIXED_WORD_SIZE
#error "HW_SPI1_FIXED_WORD_SIZE must be defined when HW_SPI1_USE_FIXED_WORD_SIZE is set!"
#endif
#endif

#ifdef HW_SPI2
#if HW_SPI2_USE_FIXED_WORD_SIZE == 1
/**
 * \brief SPI2 controller fixed word size value.
 *
 * This macro must be set by the user when \ref HW_SPI2_USE_FIXED_WORD_SIZE is set. See \ref
 * HW_SPI_WORD for the allowed values.
 */
#ifndef HW_SPI2_FIXED_WORD_SIZE
#error "HW_SPI2_FIXED_WORD_SIZE must be defined when HW_SPI2_USE_FIXED_WORD_SIZE is set!"
#endif
#endif
#endif

#ifdef HW_SPI3
#if HW_SPI3_USE_FIXED_WORD_SIZE == 1
/**
 * \brief SPI3 controller fixed word size value.
 *
 * This macro must be set by the user when \ref HW_SPI3_USE_FIXED_WORD_SIZE is set. See \ref
 * HW_SPI_WORD for the allowed values.
 */
#ifndef HW_SPI3_FIXED_WORD_SIZE
#error "HW_SPI3_FIXED_WORD_SIZE must be defined when HW_SPI3_USE_FIXED_WORD_SIZE is set!"
#endif
#endif
#endif

/*
 * ENUMERATION DEFINITIONS
 **
 */

/**
 * \brief Word length
 *
 * Used to set/get SPI_CONFIG_REG[SPI_WORD_LENGTH], 5-bits
 * Define the spi word length = 1 + SPI_WORD_LENGTH (range 4 to 32)
 * Values 3..31 => 4..32-bits
 *
 */
typedef enum {
        HW_SPI_WORD_4BIT = 3,    /**< 4 bits mode */
        HW_SPI_WORD_5BIT,        /**< 5 bits mode */
        HW_SPI_WORD_6BIT,        /**< 6 bits mode */
        HW_SPI_WORD_7BIT,        /**< 7 bits mode */
        HW_SPI_WORD_8BIT,        /**< 8 bits mode */
        HW_SPI_WORD_9BIT,        /**< 9 bits mode */
        HW_SPI_WORD_10BIT,       /**< 10 bits mode */
        HW_SPI_WORD_11BIT,       /**< 11 bits mode */
        HW_SPI_WORD_12BIT,       /**< 12 bits mode */
        HW_SPI_WORD_13BIT,       /**< 13 bits mode */
        HW_SPI_WORD_14BIT,       /**< 14 bits mode */
        HW_SPI_WORD_15BIT,       /**< 15 bits mode */
        HW_SPI_WORD_16BIT,       /**< 16 bits mode */
        HW_SPI_WORD_17BIT,       /**< 17 bits mode */
        HW_SPI_WORD_18BIT,       /**< 18 bits mode */
        HW_SPI_WORD_19BIT,       /**< 19 bits mode */
        HW_SPI_WORD_20BIT,       /**< 20 bits mode */
        HW_SPI_WORD_21BIT,       /**< 21 bits mode */
        HW_SPI_WORD_22BIT,       /**< 22 bits mode */
        HW_SPI_WORD_23BIT,       /**< 23 bits mode */
        HW_SPI_WORD_24BIT,       /**< 24 bits mode */
        HW_SPI_WORD_25BIT,       /**< 25 bits mode */
        HW_SPI_WORD_26BIT,       /**< 26 bits mode */
        HW_SPI_WORD_27BIT,       /**< 27 bits mode */
        HW_SPI_WORD_28BIT,       /**< 28 bits mode */
        HW_SPI_WORD_29BIT,       /**< 29 bits mode */
        HW_SPI_WORD_30BIT,       /**< 30 bits mode */
        HW_SPI_WORD_31BIT,       /**< 31 bits mode */
        HW_SPI_WORD_32BIT,       /**< 32 bits mode */
} HW_SPI_WORD;

/**
 * \brief Master/slave mode
 *
 * Used to set/get SPI_CONFIG_REG[SPI_SLAVE_EN], where:
 *      0 = SPI module master mode
 *      1 = SPI module slave mode
 *
 */
typedef enum {
        HW_SPI_MODE_MASTER,
        HW_SPI_MODE_SLAVE,
} HW_SPI_MODE;

/**
 * \brief Defines the SPI mode Clock Polarity and Clock Phase (CPOL, CPHA)
 *
 * Used to set/get SPI_CONFIG_REG[SPI_MODE]
 *
 */
typedef enum {
        HW_SPI_CP_MODE_0,         /**< CPOL=0, CPHA=0:
                                        New data on falling, capture on rising, Clk low in idle state */
        HW_SPI_CP_MODE_1,         /**< CPOL=0, CPHA=1:
                                        New data on rising, capture on falling, Clk low in idle state */
        HW_SPI_CP_MODE_2,         /**< CPOL=1, CPHA=0:
                                        New data on rising, capture on falling, Clk high in idle state */
        HW_SPI_CP_MODE_3,         /**< CPOL=1, CPHA=1:
                                        New data on falling, capture on rising, Clk high in idle state */
} HW_SPI_MODE_CPOL_CPHA;

/**
 * \brief Disable/enable interrupts to the CPU
 *
 */
typedef enum {
        HW_SPI_MINT_DISABLE,
        HW_SPI_MINT_ENABLE,
} HW_SPI_MINT;

/**
 * \brief Source clock's divider for the selected SPI clock frequency
 *
 * SPI_CLK = module_clk / 2*(SPI_CLK_DIV+1) when SPI_CLK_DIV not 0x7F
 * If SPI_CLK_DIV=0x7F then SPI_CLK=module_clk
 *
 */
typedef uint8_t HW_SPI_FREQ;

/**
 * \brief Clock source
 *
 */
typedef enum {
        HW_SPI_CLK_SRC_DIV_N          = 0,    /**< DIVN clock */
        HW_SPI_CLK_SRC_DIV_1          = 1,    /**< DIV1 clock */
        HW_SPI_CLK_SRC_INVALID,               /**< Any other value */
} HW_SPI_CLK_SRC;

/**
 * \brief Define the SPI master edge capture type
 *
 */
typedef enum {
        HW_SPI_MASTER_EDGE_CAPTURE_CURRENT,    /**< SPI master captures data at current clock edge */
        HW_SPI_MASTER_EDGE_CAPTURE_NEXT,       /**< SPI master captures data at next clock edge.
                                                    (only for high clock configurations) */
} HW_SPI_MASTER_EDGE_CAPTURE;

/**
 * \brief Control the CS output in master mode
 *
 */
typedef enum {
        HW_SPI_CS_NONE         = 0,         /**< None slave device selected */
        HW_SPI_CS_0            = 1,         /**< Selected slave device connected to GPIO with FUNC_MODE = SPI_CS0 */
        HW_SPI_CS_1            = 2,         /**< Selected slave device connected to GPIO with FUNC_MODE = SPI_CS1 */
        HW_SPI_CS_GPIO         = 4,         /**< Selected slave device connected to GPIO with FUNC_MODE = GPIO */
} HW_SPI_CS_MODE;

/**
 * \brief Define the SPI RX/TX FIFO threshold level in bytes
 *
 * Valid values are:
 *      SPI and SPI2: 0 to 32 bytes
 *      SPI3: 0 to 4 bytes
 *
 */
typedef enum {
        HW_SPI_FIFO_LEVEL0,     /**< SPI RX/TX FIFO threshold level is 0 bytes */
        HW_SPI_FIFO_LEVEL1,     /**< SPI RX/TX FIFO threshold level is 1 bytes */
        HW_SPI_FIFO_LEVEL2,     /**< SPI RX/TX FIFO threshold level is 2 bytes */
        HW_SPI_FIFO_LEVEL3,     /**< SPI RX/TX FIFO threshold level is 3 bytes */
        HW_SPI_FIFO_LEVEL4,     /**< SPI RX/TX FIFO threshold level is 4 bytes */
        HW_SPI_FIFO_LEVEL5,     /**< SPI RX/TX FIFO threshold level is 5 bytes */
        HW_SPI_FIFO_LEVEL6,     /**< SPI RX/TX FIFO threshold level is 6 bytes */
        HW_SPI_FIFO_LEVEL7,     /**< SPI RX/TX FIFO threshold level is 7 bytes */
        HW_SPI_FIFO_LEVEL8,     /**< SPI RX/TX FIFO threshold level is 8 bytes */
        HW_SPI_FIFO_LEVEL9,     /**< SPI RX/TX FIFO threshold level is 9 bytes */
        HW_SPI_FIFO_LEVEL10,     /**< SPI RX/TX FIFO threshold level is 10 bytes */
        HW_SPI_FIFO_LEVEL11,     /**< SPI RX/TX FIFO threshold level is 11 bytes */
        HW_SPI_FIFO_LEVEL12,     /**< SPI RX/TX FIFO threshold level is 12 bytes */
        HW_SPI_FIFO_LEVEL13,     /**< SPI RX/TX FIFO threshold level is 13 bytes */
        HW_SPI_FIFO_LEVEL14,     /**< SPI RX/TX FIFO threshold level is 14 bytes */
        HW_SPI_FIFO_LEVEL15,     /**< SPI RX/TX FIFO threshold level is 15 bytes */
        HW_SPI_FIFO_LEVEL16,     /**< SPI RX/TX FIFO threshold level is 16 bytes */
        HW_SPI_FIFO_LEVEL17,     /**< SPI RX/TX FIFO threshold level is 17 bytes */
        HW_SPI_FIFO_LEVEL18,     /**< SPI RX/TX FIFO threshold level is 18 bytes */
        HW_SPI_FIFO_LEVEL19,     /**< SPI RX/TX FIFO threshold level is 19 bytes */
        HW_SPI_FIFO_LEVEL20,     /**< SPI RX/TX FIFO threshold level is 20 bytes */
        HW_SPI_FIFO_LEVEL21,     /**< SPI RX/TX FIFO threshold level is 21 bytes */
        HW_SPI_FIFO_LEVEL22,     /**< SPI RX/TX FIFO threshold level is 22 bytes */
        HW_SPI_FIFO_LEVEL23,     /**< SPI RX/TX FIFO threshold level is 23 bytes */
        HW_SPI_FIFO_LEVEL24,     /**< SPI RX/TX FIFO threshold level is 24 bytes */
        HW_SPI_FIFO_LEVEL25,     /**< SPI RX/TX FIFO threshold level is 25 bytes */
        HW_SPI_FIFO_LEVEL26,     /**< SPI RX/TX FIFO threshold level is 26 bytes */
        HW_SPI_FIFO_LEVEL27,     /**< SPI RX/TX FIFO threshold level is 27 bytes */
        HW_SPI_FIFO_LEVEL28,     /**< SPI RX/TX FIFO threshold level is 28 bytes */
        HW_SPI_FIFO_LEVEL29,     /**< SPI RX/TX FIFO threshold level is 29 bytes */
        HW_SPI_FIFO_LEVEL30,     /**< SPI RX/TX FIFO threshold level is 30 bytes */
        HW_SPI_FIFO_LEVEL31,     /**< SPI RX/TX FIFO threshold level is 31 bytes */
        HW_SPI_FIFO_LEVEL32,     /**< SPI RX/TX FIFO threshold level is 32 bytes */
} HW_SPI_FIFO_TL;

/**
 * \brief FIFO mode
 *
 */
typedef enum {
        HW_SPI_FIFO_RX_TX       = 0,    /**< Bidirectional mode. */
        HW_SPI_FIFO_RX_ONLY     = 1,    /**< Read only mode. */
        HW_SPI_FIFO_TX_ONLY     = 2,    /**< Write only mode. */
        HW_SPI_FIFO_NONE        = 3,    /**< Backwards compatible mode. */
} HW_SPI_FIFO;

/**
 * \brief SPI chip-select pin definition
 *
 */
typedef struct
{
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;
} SPI_Pad;

#if (HW_SPI_DMA_SUPPORT == 1)
#include "hw_dma.h"

/**
 * \brief SPI DMA priority configuration
 *
 * \note DMA channel priorities are configured to their default values
 * when use_prio = false
 */
typedef hw_dma_periph_prio_t hw_spi_dma_prio_t;
#endif /* HW_SPI_DMA_SUPPORT */

/**
 * \brief SPI configuration
 *
 */
typedef struct
{
        SPI_Pad                 cs_pad;         /**< Configure SPI chip-select pin */
        HW_SPI_WORD             word_mode;      /**< Configure SPI word length */
        HW_SPI_MODE             smn_role;       /**< Configure SPI master/slave mode */
        HW_SPI_MODE_CPOL_CPHA   cpol_cpha_mode; /**< Configure clock polarity and phase */

        DEPRECATED_MSG("API no longer supported, mint_mode is not used.")
        HW_SPI_MINT             mint_mode;
        HW_SPI_FREQ             xtal_freq;      /**< Configure the clock divider applied to the clock source which defines the SPI clock frequency */
        HW_SPI_FIFO             fifo_mode;      /**< Configure SPI fifo mode at initialization */
        uint8_t                 disabled;       /**< Configure SPI (enable/disable) at initialization */
        HW_SPI_CS_MODE          spi_cs;         /**< Configure the CS output in master mode */
        HW_SPI_FIFO_TL          rx_tl;          /**< Configure FIFO_RX_TL. Note: in case DMA is used, rx_tl should be 0 */
        HW_SPI_FIFO_TL          tx_tl;          /**< Configure FIFO_TX_TL */
        bool                    swap_bytes;     /**< Configure to change endianness in APB interface or not */
        bool                    select_divn;    /**< Configure DIVN */
#if (HW_SPI_DMA_SUPPORT == 1)
        uint8_t                 use_dma;                /**< Configure using DMA or not */
        HW_DMA_CHANNEL          rx_dma_channel:8;       /**< Configure Rx DMA channel */
        HW_DMA_CHANNEL          tx_dma_channel:8;       /**< Configure Tx DMA channel */
        hw_spi_dma_prio_t       dma_prio;               /**< Configure DMA priority */
#endif /* HW_SPI_DMA_SUPPORT */
} spi_config;

//===================== Read/Write functions ===================================

/**
 * \brief Write a value to an SPI register field
 *
 * \param [in] id SPI controller instance
 * \param [in] reg the SPI register
 * \param [in] field the SPI register field
 * \param [in] val value to be written
 *
 * \sa HW_SPI_REG_GETF
 *
 */
#define HW_SPI_REG_SETF(id, reg, field, val) \
        SBA(id)->reg = ((SBA(id)->reg & ~(SPI_##reg##_##field##_Msk)) | \
        ((SPI_##reg##_##field##_Msk) & ((val) << (SPI_##reg##_##field##_Pos))))

/**
 * \brief Get the value of an SPI register field
 *
 * \param [in] id SPI controller instance
 * \param [in] reg the SPI register
 * \param [in] field the SPI register field
 *
 * \sa HW_SPI_REG_SETF
 *
 */
#define HW_SPI_REG_GETF(id, reg, field) \
        ((SBA(id)->reg & (SPI_##reg##_##field##_Msk)) >> (SPI_##reg##_##field##_Pos))

/**
 * \brief Assertion for SPI registers configuration
 *
 * SPI registers can be configured only when the SPI clock is enabled, i.e.:
 * CLK_SNC_REG[SPI_ENABLE] = 1 for id=HW_SPI1, or
 * CLK_SNC_REG[SPI2_ENABLE] = 1 for id=HW_SPI2, or
 * CLK_SYS_REG[SPI3_ENABLE] = 1 for id=HW_SPI3
 *
 * \param [in] id SPI controller instance
 *
 * \sa HW_SPI_ASSERT
 *
 */
__STATIC_INLINE void HW_SPI_ASSERT(const HW_SPI_ID id)
{
        if (id == HW_SPI1) {
                ASSERT_WARNING(REG_GETF(CRG_SNC, CLK_SNC_REG, SPI_ENABLE) == true);
        } else if (id == HW_SPI2) {
                ASSERT_WARNING(REG_GETF(CRG_SNC, CLK_SNC_REG, SPI2_ENABLE) == true);
        }  else if (id == HW_SPI3) {
                ASSERT_WARNING(REG_GETF(CRG_SYS, CLK_SYS_REG, SPI3_ENABLE) == true);
        } else {
                ASSERT_WARNING(0);
        }
};

/**
 * \brief Read 4 to 16-bits from RX FIFO
 *
 * Word size must be setup before to 4 to 16-bits.
 * This function should be called only when FIFO is not empty.
 * Call hw_spi_get_fifo_status_reg_rx_empty() before using this.
 * MSBits are zero if word length is smaller than 16-bits.
 *
 * \param [in] id SPI controller instance
 *
 * \return read data
 *
 * \sa hw_spi_get_fifo_status_reg_rx_empty
 *
 */
__STATIC_INLINE uint16_t hw_spi_fifo_read16(HW_SPI_ID id)
{
        // Read data from the SPI RX register or RX FIFO
        return (uint16_t) SBA(id)->SPI_FIFO_READ_REG;
}

/**
 * \brief Write 4 to 16-bits to TX FIFO.
 *
 * Word size must be setup before to 4 to 16-bits.
 * This function should be called only when FIFO is not full.
 * Call hw_spi_is_tx_fifo_full() before using this.
 * MSBits are ignored if word length is smaller than 16-bits.
 *
 * \param [in] id SPI controller instance
 * \param [in] data data to be written
 *
 * \sa hw_spi_is_tx_fifo_full
 *
 */
__STATIC_INLINE void hw_spi_fifo_write16(HW_SPI_ID id, uint16_t data)
{
        SBA(id)->SPI_FIFO_WRITE_REG = data;
}

/**
 * \brief Read 4 to 8-bits from RX FIFO.
 *
 * Word size must be setup before to 4 to 8-bits.
 * This function should be called only when FIFO is not empty.
 * Call hw_spi_get_fifo_status_reg_rx_empty() before using this.
 * MSBits are zero if word length is smaller than 8-bits.
 *
 * \param [in] id SPI controller instance
 *
 * \return read data
 *
 * \sa hw_spi_get_fifo_status_reg_rx_empty
 *
 */
__STATIC_INLINE uint8_t hw_spi_fifo_read8(HW_SPI_ID id)
{
        // Read byte from the SPI RX register or RX FIFO
        return (uint8_t) SBA(id)->SPI_FIFO_READ_REG;
}

/**
 * \brief Write 4 to 8-bits to TX FIFO.
 *
 * Word size must be setup before to 4 to 8-bits.
 * This function should be called only when FIFO is not full.
 * Call hw_spi_is_tx_fifo_full() before using this.
 * MSBits are ignored if word length is smaller than 8-bits.
 *
 * \param [in] id SPI controller instance
 * \param [in] data data to be written
 *
 * \sa hw_spi_is_tx_fifo_full
 *
 */
__STATIC_INLINE void hw_spi_fifo_write8(HW_SPI_ID id, uint8_t data)
{
        // Write byte to the SPI TX register or TX FIFO
        SBA(id)->SPI_FIFO_WRITE_REG = data;
}

/**
 * \brief Read 4 to 32-bits from RX FIFO.
 *
 * Word size must be setup before to 4 to 32-bits.
 * This function should be called only when FIFO is not empty.
 * Call hw_spi_get_fifo_status_reg_rx_empty() before using this.
 * MSBits are zero if word length is smaller than 32-bits.
 *
 * \param [in] id SPI controller instance
 *
 * \return read data
 *
 * \sa hw_spi_get_fifo_status_reg_rx_empty
 *
 */
__STATIC_INLINE uint32_t hw_spi_fifo_read32(HW_SPI_ID id)
{
        return SBA(id)->SPI_FIFO_READ_REG;
}

/**
 * \brief Write 4 to 32-bits to TX FIFO.
 *
 * Word size must be setup before to 4 to 32-bits.
 * This function should be called only when FIFO is not full.
 * Call hw_spi_is_tx_fifo_full() before using this.
 * MSBits are ignored if word length is smaller than 32-bits.
 *
 * \param [in] id SPI controller instance
 * \param [in] data data to be written
 *
 * \sa hw_spi_is_tx_fifo_full
 *
 */
__STATIC_INLINE void hw_spi_fifo_write32(HW_SPI_ID id, uint32_t data)
{
        SBA(id)->SPI_FIFO_WRITE_REG = data;
}

/**
 * \brief Writes/reads 4 to 16 bits to the SPI
 *
 * Function sends word to SPI and reads back data on same clock.
 * Word size must be setup before to 4 to 16-bits.
 * Data is sent in big-endian mode, MSB goes first.
 * MSBits are ignored if word length is smaller than 16-bits.
 *
 * \note Use hw_spi_set_txbuffer_force_reg first, when device is slave
 *
 * \param [in] id SPI controller interface
 * \param [in] val value to send
 *
 * \return value read from MISO line
 *
 * \sa hw_spi_writeread32, hw_spi_set_txbuffer_force_reg
 *
 */
uint16_t hw_spi_writeread(HW_SPI_ID id, uint16_t val);

/**
 * \brief Writes/reads 4 to 32 bits to the SPI
 *
 * Function sends word to SPI and reads back data on same clock.
 * Word size must be setup before to 4 to 32-bits.
 * Data is sent in big-endian mode, MSB goes first.
 * MSBits are ignored if word length is smaller than 32-bits.
 *
 * \note Use hw_spi_set_txbuffer_force_reg first, when device is slave
 *
 * \param [in] id SPI controller interface
 * \param [in] val value to send
 *
 * \return value read from MISO line
 *
 * \sa hw_spi_writeread, hw_spi_set_txbuffer_force_reg
 *
 */
uint32_t hw_spi_writeread32(HW_SPI_ID id, uint32_t val);

/**
 * \brief Write and reads array of bytes through SPI
 *
 * Initiates SPI transmission, data is sent and received at the same time.
 * If no callback is provided this function wait for transfer to finish.
 * If callback is provided function sets up transfer in interrupt or dma mode and ends immediately.
 * In this case data pointed by in_buf and out_buf should not be touched till callback is called.
 *
 * \note In interrupt mode, the SPI_STATUS_TX_EMPTY event triggers the SPI handler for the first time only,
 * and the TX FIFO is written until it is full.
 * The SPI_STATUS_TX_EMPTY event is disabled and thus the TX_TL is used for the first time only.
 * The SPI_STATUS_RX_FULL event triggers the handler from now on according to RX_TL, where
 * the master/slave starts reading the RX_FIFO until it is empty and writing TX_FIFO until full.
 * This is repeated until len bytes are read into in_buf.
 *
 * \param [in] id SPI controller instance
 * \param [in] out_buf data to send
 * \param [out] in_buf buffer for incoming data
 * \param [in] len data length in bytes
 * \param [in] cb callback to call after transfer is finished
 * \param [in] user_data parameter for callback
 *
 * \note Supplied buffer addresses and lengths must be SPI-word-aligned.
*/
void hw_spi_writeread_buf(HW_SPI_ID id, const uint8_t *out_buf, uint8_t *in_buf, uint16_t len,
                                                            hw_spi_tx_callback cb, void *user_data);

/**
 * \brief Write array of bytes to SPI
 *
 * Initiates SPI transmission, no data is received (Write only mode)
 * If no callback is provided this function wait for transfer to finish.
 * If callback is provided function sets up transfer and ends immediately.
 * In this case data pointed by out_buf should not be touched till callback is
 * called.
 *
 * \param [in] id SPI controller instance
 * \param [in] out_buf data to send
 * \param [in] len data length in bytes
 * \param [in] cb callback to call after transfer is finished
 * \param [in] user_data parameter for callback
 *
 * \note In slave mode, the first word to be transmitted should be written to
 * the TX buffer and the remaining words are written to the TX FIFO.
 *
 * The mechanism that fetches data into the TX fifo and from TX fifo to TX buffer is as follows:
 *
 * DMA mode:
 * When data is to be transmitted in Slave mode using DMA, the first word is written to
 * SPI_TXBUFFER_FORCE_REG and the DMA is programmed to send the remaining words.
 * When the number of bytes in the TX fifo is less or equal to the TX threshold level
 * set by the user (SPI_TX_TL), an SPI_STATUS_TX_EMPTY event triggers the DMA to
 * fetch new data into the TX FIFO. When the last byte is added in the TX fifo,
 * the dma has finished, the TX dma callback is called and ad_spi_write() returns.
 * This means that for the TX Path, there is no signal notifying about the actual transmission
 * of the last byte on the bus.
 *
 * Interrupt mode:
 * When the number of bytes in the TX fifo is less or equal to the TX threshold level
 * set by the user (SPI_TX_TL), an SPI_STATUS_TX_EMPTY event triggers the spi interrupt handler,
 * which adds new data in the TX fifo. The user callback is called when the last byte
 * is added in the TX fifo.

 * In the special case where only one word needs to be sent,
 * DMA cannot not be enabled to send zero words, although DMA may have been requested
 * by the user during driver initialization. Instead, interrupt mode will be enabled
 * for receiving an asynchronous notification and eventually call the user callback
 * as required by the driver API.
 *
 * In the special case where two words needs to be sent, DMA will be enabled.
 * The first word is written to SPI_TXBUFFER_FORCE_REG and the DMA is programmed to send
 * the remaining word. The first time, the TX fifo is always empty i.e. it always has less or
 * equal bytes to the TX threshold level set by the user (SPI_TX_TL).
 * Therefore, an SPI_STATUS_TX_EMPTY event triggers the DMA to fetch a new word into the TX FIFO.
 * When the last and only one word is added in the TX fifo, the dma has finished and
 * the TX dma callback is called.
 *
 * \note Supplied buffer address and length must be non-zero and SPI-word-aligned.
*/
void hw_spi_write_buf(HW_SPI_ID id, const uint8_t *out_buf, uint16_t len,
                                                            hw_spi_tx_callback cb, void *user_data);

/**
 * \brief Reads array of bytes through SPI
 *
 * Initiates SPI read transfer.
 * If no callback is provided this function wait for transfer to finish.
 * If callback is provided function sets up transfer in interrupt or dma mode and ends immediately.
 * In this case data pointed by in_buf and out_buf should not be touched till callback is called.
 *
 * \note In master mode, the TX path is enabled as well as the RX path.
 * The master has to write dummy data, thus giving a clock to slave and read data from slave.
 *
 * \note In master interrupt mode, the SPI_STATUS_TX_EMPTY event triggers the SPI handler for the first time only,
 * and the TX FIFO is written (dummy writes) until it is full.
 * The SPI_STATUS_TX_EMPTY event is disabled and thus the TX_TL is used for the first time only.
 * The SPI_STATUS_RX_FULL event triggers the handler from now on according to RX_TL, where
 * the master starts reading the RX_FIFO until it is empty and writing TX_FIFO until full.
 * This is repeated until len bytes are read into in_buf.
 *
 * \note In slave interrupt mode, the SPI_STATUS_RX_FULL event triggers the SPI handler, according to the RX_TL,
 * where the slave starts reading the RX_FIFO until it is empty.
 * This is repeated until len bytes are read into in_buf.
 *
 * \param [in] id SPI controller instance
 * \param [out] in_buf buffer for incoming data
 * \param [in] len data length in bytes
 * \param [in] cb callback to call after transfer is finished
 * \param [in] user_data parameter for callback
 *
 * \note Supplied buffer address and length must be SPI-word-aligned.
 */
void hw_spi_read_buf(HW_SPI_ID id, uint8_t *in_buf, uint16_t len,
                                                            hw_spi_tx_callback cb, void *user_data);

/**
 * \brief Get SPI fifo depth in bytes
 *
 * \param [in] id       SPI controller instance
 * \return HW_SPI_FIFO_TL
 *
 */
__STATIC_INLINE HW_SPI_FIFO_TL hw_spi_get_fifo_depth_in_bytes(const HW_SPI_ID id)
{
        return (id == HW_SPI3) ? HW_SPI_FIFO_LEVEL4 : HW_SPI_FIFO_LEVEL32;
}

/*
 * Low Level Register Access Functions
 **
 */

/***************** SPI_CTRL_REG Functions *****************/

/**
 * \brief Set SPI Control Register Value
 *
 * \param [in] id       SPI controller instance
 * \param [in] val      SPI Control Register Value
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg(HW_SPI_ID id, uint8_t val)
{
        SBA(id)->SPI_CTRL_REG = val;
}

/**
 * \brief Get SPI Control Register Value
 *
 * \param [in] id       SPI controller instance
 *
 * \return SPI Control Register Value
 *
 */
__STATIC_INLINE uint8_t hw_spi_get_ctrl_reg(HW_SPI_ID id)
{
        return (uint8_t) SBA(id)->SPI_CTRL_REG;
}

/**
 * \brief Set SPI Control Register Value to clear SPI enable
 *
 * \param [in] id       SPI controller instance
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_clear_enable(HW_SPI_ID id)
{
        uint32_t ctrl = SBA(id)->SPI_CTRL_REG;

        REG_SET_FIELD(SPI, SPI_CTRL_REG, SPI_EN, ctrl, false);
        REG_SET_FIELD(SPI, SPI_CTRL_REG, SPI_TX_EN, ctrl, false);
        REG_SET_FIELD(SPI, SPI_CTRL_REG, SPI_RX_EN, ctrl, false);
        REG_SET_FIELD(SPI, SPI_CTRL_REG, SPI_DMA_TX_EN, ctrl, false);
        REG_SET_FIELD(SPI, SPI_CTRL_REG, SPI_DMA_RX_EN, ctrl, false);

        SBA(id)->SPI_CTRL_REG = ctrl;
}

/**
 * \brief Set SPI_EN in Control Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_enable       false: Disable, true: Enable
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_spi_en(HW_SPI_ID id, bool spi_enable)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_EN, spi_enable);
}

/**
 *
 * \brief Get SPI_EN from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_ctrl_reg_spi_en(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_EN);
}

/**
 *
 * \brief Set SPI_TX_EN in Control Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_tx_enable    false: Disable, true: Enable
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_tx_en(HW_SPI_ID id, bool spi_tx_enable)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_TX_EN, spi_tx_enable);
}

/**
 *
 * \brief Get SPI_TX_EN from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_ctrl_reg_tx_en(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_TX_EN);
}

/**
 *
 * \brief Set SPI_RX_EN in Control Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_rx_enable    false: Disable, true: Enable
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_rx_en(HW_SPI_ID id, bool spi_rx_enable)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_RX_EN, spi_rx_enable);
}

/**
 *
 * \brief Get SPI_RX_EN from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_ctrl_reg_rx_en(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_RX_EN);
}

/**
 *
 * \brief Set SPI_DMA_TX_EN in Control Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_dma_tx_enable false: Disable, true: Enable
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_dma_tx_en(HW_SPI_ID id, bool spi_dma_tx_enable)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_DMA_TX_EN, spi_dma_tx_enable);
}

/**
 *
 * \brief Get SPI_DMA_TX_EN from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_ctrl_reg_dma_tx_en(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_DMA_TX_EN);
}

/**
 *
 * \brief Set SPI_DMA_RX_EN in Control Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_dma_rx_enable false: Disable, true: Enable
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_dma_rx_en(HW_SPI_ID id, bool spi_dma_rx_enable)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_DMA_RX_EN, spi_dma_rx_enable);
}

/**
 *
 * \brief Get SPI_DMA_RX_EN from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_ctrl_reg_dma_rx_en(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_DMA_RX_EN);
}

/**
 *
 * \brief Set SPI_FIFO_RESET in Control Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_fifo_reset false: Disable, true: Enable
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_fifo_reset(HW_SPI_ID id, bool spi_fifo_reset)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_FIFO_RESET, spi_fifo_reset);
}

/**
 *
 * \brief Get SPI_FIFO_RESET from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_ctrl_reg_fifo_reset(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_FIFO_RESET);
}

/**
 *
 * \brief Set SPI_CAPTURE_AT_NEXT_EDGE in Control Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] capture_next_edge
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_capture_next_edge(HW_SPI_ID id, HW_SPI_MASTER_EDGE_CAPTURE capture_next_edge)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_CAPTURE_AT_NEXT_EDGE, capture_next_edge);
}

/**
 *
 * \brief Get SPI_CAPTURE_AT_NEXT_EDGE from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return SPI_MASTER_EDGE_CAPTURE_CFG
 *
 */
__STATIC_INLINE HW_SPI_MASTER_EDGE_CAPTURE hw_spi_get_ctrl_reg_capture_next_edge(HW_SPI_ID id)
{
        return (HW_SPI_MASTER_EDGE_CAPTURE) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_CAPTURE_AT_NEXT_EDGE);
}

/**
 *
 * \brief Set SPI_SWAP_BYTES in Control Register
 *
 * In case of 8bit interface, SPI can be configured to change word endianness to off load the bus.
 * It applies to 16-bit or 32-bit words. No use in case of 8-bit words.
 *
 * \note The name "SPI_SWAP_BYTES" comes from the former implementation of the SPI block
 *       which supported 16-bit words only.
 *
 * \param [in] id               SPI controller instance
 * \param [in] swap_bytes       false: normal operation
 *                              true: change endianness in APB interface
 *
 */
__STATIC_INLINE void hw_spi_set_ctrl_reg_swap_bytes(HW_SPI_ID id, bool swap_bytes)
{
        HW_SPI_REG_SETF(id, SPI_CTRL_REG, SPI_SWAP_BYTES, swap_bytes);
}

/**
 *
 * \brief Get SPI_SWAP_BYTES from Control Register
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_ctrl_reg_swap_bytes(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_CTRL_REG, SPI_SWAP_BYTES);
}

/***************** SPI_CONFIG_REG Functions *****************/

/**
 *
 * \brief Set SPI Configuration Register Value.
 *
 * \note SPI3 should be used only as master
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_config_reg   SPI Configuration Register Value
 *
 */
__STATIC_INLINE void hw_spi_set_config_reg(HW_SPI_ID id, uint8_t spi_config_reg)
{
#ifdef HW_SPI3
        ASSERT_WARNING(!((id == HW_SPI3) &&
                         (((spi_config_reg & SPI_SPI_CONFIG_REG_SPI_SLAVE_EN_Msk) >>
                                SPI_SPI_CONFIG_REG_SPI_SLAVE_EN_Pos) == HW_SPI_MODE_SLAVE)));
#endif
        SBA(id)->SPI_CONFIG_REG = spi_config_reg;
}

/**
 *
 * \brief Get SPI Configuration Register Value.
 *
 * \param [in] id               SPI controller instance
 * \return uint8_t              SPI Configuration Register Value
 *
 */
__STATIC_INLINE uint8_t hw_spi_get_config_reg(HW_SPI_ID id)
{
        return (uint8_t) SBA(id)->SPI_CONFIG_REG;
}

/**
 *
 * \brief Set SPI_MODE in Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_cp           HW_SPI_MODE_CPOL_CPHA
 *
 */
__STATIC_INLINE void hw_spi_set_config_reg_spi_mode(HW_SPI_ID id, HW_SPI_MODE_CPOL_CPHA spi_cp)
{
        HW_SPI_REG_SETF(id, SPI_CONFIG_REG, SPI_MODE, spi_cp);
}

/**
 *
 * \brief Get SPI_MODE from Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \return HW_SPI_MODE_CPOL_CPHA
 *
 */
__STATIC_INLINE HW_SPI_MODE_CPOL_CPHA hw_spi_get_config_reg_spi_mode(HW_SPI_ID id)
{
        return (HW_SPI_MODE_CPOL_CPHA) HW_SPI_REG_GETF(id, SPI_CONFIG_REG, SPI_MODE);
}

/**
 *
 * \brief Set SPI_WORD_LENGTH in Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_wsz          SPI Word size - 1 in bits
 *
 */
__STATIC_INLINE void hw_spi_set_config_reg_word_len(HW_SPI_ID id, HW_SPI_WORD spi_wsz)
{
        HW_SPI_REG_SETF(id, SPI_CONFIG_REG, SPI_WORD_LENGTH, spi_wsz);
}

/**
 *
 * \brief Get SPI_WORD_LENGTH from Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \return HW_SPI_WORD          SPI Word size - 1 in bits
 *
 */
__STATIC_INLINE HW_SPI_WORD hw_spi_get_config_reg_word_len(HW_SPI_ID id)
{
        return (HW_SPI_WORD) HW_SPI_REG_GETF(id, SPI_CONFIG_REG, SPI_WORD_LENGTH);
}

/**
 *
 * \brief Set SPI_SLAVE_EN in Configuration Register
 *
 * \note SPI3 should be used only as master
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_ms
 *
 */
__STATIC_INLINE void hw_spi_set_config_reg_slave_en(HW_SPI_ID id, HW_SPI_MODE spi_ms)
{
#ifdef HW_SPI3
        ASSERT_WARNING(((id == HW_SPI3) && (spi_ms == HW_SPI_MODE_SLAVE)) == false);
#endif
        HW_SPI_REG_SETF(id, SPI_CONFIG_REG, SPI_SLAVE_EN, spi_ms);
}

/**
 *
 * \brief Get SPI Master/Slave mode from Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \return HW_SPI_MODE
 *
 */
__STATIC_INLINE HW_SPI_MODE hw_spi_get_config_reg_slave_en(HW_SPI_ID id)
{
        return (HW_SPI_MODE) HW_SPI_REG_GETF(id, SPI_CONFIG_REG, SPI_SLAVE_EN);
}

/***************** SPI_CLOCK_REG Functions *****************/

/**
 *
 * \brief Check if the SPI clock is enabled.
 *
 * \param [in] id               SPI controller instance
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_spi_get_clock_en(const HW_SPI_ID id)
{
        if (id == HW_SPI1) {
                return (bool) REG_GETF(CRG_SNC, CLK_SNC_REG, SPI_ENABLE);
        } else if (id == HW_SPI2) {
                return (bool) REG_GETF(CRG_SNC, CLK_SNC_REG, SPI2_ENABLE);
        } else if (id == HW_SPI3) {
                return (bool) REG_GETF(CRG_SYS, CLK_SYS_REG, SPI3_ENABLE);
        } else {
                ASSERT_ERROR(0);
                return false;
        }
}

/**
 *
 * \brief Set SPI Clock Register Value.
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_clock_reg    SPI Clock Register Value
 *
 */
__STATIC_INLINE void hw_spi_set_clock_reg(HW_SPI_ID id, uint8_t spi_clock_reg)
{
        SBA(id)->SPI_CLOCK_REG = spi_clock_reg;
}

/**
 *
 * \brief Get SPI Clock Register Value.
 *
 * \param [in] id               SPI controller instance
 * \return uint8_t              SPI Clock Register Value
 *
 */
__STATIC_INLINE uint8_t hw_spi_get_clock_reg(HW_SPI_ID id)
{
        return SBA(id)->SPI_CLOCK_REG;
}

/**
 *
 * \brief Set SPI_CLK_DIV in Clock Register Applicable only in master mode. Defines
 *        the spi clock frequency in master only mode.
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_clk_div      SPI_CLK = module_clk / 2 * (SPI_CLK_DIV + 1)
 *                              when SPI_CLK_DIV is not 0x7F.
 *                              If SPI_CLK_DIV = 0x7F then SPI_CLK = module_clk
 *
 */
__STATIC_INLINE void hw_spi_set_clock_reg_clk_div(HW_SPI_ID id, HW_SPI_FREQ spi_clk_div)
{
        ASSERT_WARNING(spi_clk_div <= SPI_SPI_CLOCK_REG_SPI_CLK_DIV_Msk);
        HW_SPI_REG_SETF(id, SPI_CLOCK_REG, SPI_CLK_DIV, spi_clk_div);
}

/**
 *
 * \brief Get SPI_CLK_DIV from Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \return spi_clk_div
 *
 */
__STATIC_INLINE HW_SPI_FREQ hw_spi_get_clock_reg_clk_div(HW_SPI_ID id)
{
        return (HW_SPI_FREQ) HW_SPI_REG_GETF(id, SPI_CLOCK_REG, SPI_CLK_DIV);
}

/***************** SPI_FIFO_CONFIG_REG Functions *****************/

/**
 *
 * \brief Set SPI_TX_TL in FIFO Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_tx_tl        Transmit FIFO threshold level in bytes. Control the level
 *                              of bytes in fifo that triggers the TX_EMPTY interrupt.
 *                              IRQ is occurred when fifo level is less or equal to
 *                              spi_tx_tl.
 *
 * \note FIFO threshold level is 0 to 32 bytes for SPI and SPI2, and 0 to 4 bytes for SPI3
 */
__STATIC_INLINE void hw_spi_set_fifo_config_reg_tx_tl(HW_SPI_ID id, HW_SPI_FIFO_TL spi_tx_tl)
{
        ASSERT_WARNING(spi_tx_tl <= hw_spi_get_fifo_depth_in_bytes(id));
        HW_SPI_REG_SETF(id, SPI_FIFO_CONFIG_REG, SPI_TX_TL, spi_tx_tl);
}

/**
 *
 * \brief Get SPI_TX_TL from FIFO Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \return spi_tx_tl
 *
 */
__STATIC_INLINE HW_SPI_FIFO_TL hw_spi_get_fifo_config_reg_tx_tl(HW_SPI_ID id)
{
        return (HW_SPI_FIFO_TL) HW_SPI_REG_GETF(id, SPI_FIFO_CONFIG_REG, SPI_TX_TL);
}

/**
 *
 * \brief Set SPI_RX_TL in FIFO Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] spi_rx_tl        Receive FIFO threshold level in bytes. Control the level
 *                              of bytes in fifo that triggers the RX_FULL interrupt.
 *                              IRQ is occurred when fifo level is less or equal to
 *                              spi_rx_tl + 1.
 *
 * \note FIFO threshold level is 0 to 31 bytes for SPI and SPI2, and 0 to 3 bytes for SPI3
 */
__STATIC_INLINE void hw_spi_set_fifo_config_reg_rx_tl(HW_SPI_ID id, HW_SPI_FIFO_TL spi_rx_tl)
{
        ASSERT_WARNING(spi_rx_tl < hw_spi_get_fifo_depth_in_bytes(id));
        HW_SPI_REG_SETF(id, SPI_FIFO_CONFIG_REG, SPI_RX_TL, spi_rx_tl);
}

/**
 *
 * \brief Get SPI_RX_TL from FIFO Configuration Register
 *
 * \param [in] id               SPI controller instance
 * \return spi_rx_tl
 *
 */
__STATIC_INLINE HW_SPI_FIFO_TL hw_spi_get_fifo_config_reg_rx_tl(HW_SPI_ID id)
{
        return (HW_SPI_FIFO_TL) HW_SPI_REG_GETF(id, SPI_FIFO_CONFIG_REG, SPI_RX_TL);
}


/***************** SPI_IRQ_MASK_REG Functions *****************/
/**
 *
 * \brief Set SPI_IRQ_MASK_TX_EMPTY in IRQ Mask Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] irq_tx_empty_en  HW_SPI_MINT.
 *
 */
__STATIC_INLINE void hw_spi_set_irq_mask_reg_tx_empty_en(HW_SPI_ID id, HW_SPI_MINT irq_tx_empty_en)
{
        HW_SPI_REG_SETF(id, SPI_IRQ_MASK_REG, SPI_IRQ_MASK_TX_EMPTY, irq_tx_empty_en);
}

/**
 *
 * \brief Get SPI_IRQ_MASK_TX_EMPTY from IRQ Mask Register
 *
 * \param [in] id               SPI controller instance
 * \return HW_SPI_MINT.
 *
 */
__STATIC_INLINE HW_SPI_MINT hw_spi_get_irq_mask_reg_tx_empty_en(HW_SPI_ID id)
{
        return (HW_SPI_MINT) HW_SPI_REG_GETF(id, SPI_IRQ_MASK_REG, SPI_IRQ_MASK_TX_EMPTY);
}

/**
 *
 * \brief Set SPI_IRQ_MASK_RX_FULL in IRQ Mask Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] irq_rx_full_en   HW_SPI_MINT.
 *
 */
__STATIC_INLINE void hw_spi_set_irq_mask_reg_rx_full_en(HW_SPI_ID id, HW_SPI_MINT irq_rx_full_en)
{
        HW_SPI_REG_SETF(id, SPI_IRQ_MASK_REG, SPI_IRQ_MASK_RX_FULL, irq_rx_full_en);
}

/**
 *
 * \brief Get SPI_IRQ_MASK_RX_FULL from IRQ Mask Register
 *
 * \param [in] id               SPI controller instance
 * \return HW_SPI_MINT.
 *
 */
__STATIC_INLINE HW_SPI_MINT hw_spi_get_irq_mask_reg_rx_full_en(HW_SPI_ID id)
{
        return (HW_SPI_MINT) HW_SPI_REG_GETF(id, SPI_IRQ_MASK_REG, SPI_IRQ_MASK_RX_FULL);
}


/***************** SPI_STATUS_REG Functions *****************/

/**
 *
 * \brief Get SPI TX FIFO Empty status from Status Register
 *
 * \param [in] id               SPI controller instance
 * \return bool.
 *
 */
__STATIC_INLINE bool hw_spi_get_status_reg_tx_fifo_empty(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_STATUS_REG, SPI_STATUS_TX_EMPTY);
}

/**
 *
 * \brief Get SPI RX FIFO Full status from Status Register
 *
 * \param [in] id               SPI controller instance
 * \return bool.
 *
 */
__STATIC_INLINE bool hw_spi_get_status_reg_rx_fifo_full(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_STATUS_REG, SPI_STATUS_RX_FULL);
}


/***************** SPI_FIFO_STATUS_REG Functions *****************/

/**
 *
 * \brief Get SPI FIFO status.
 *
 * \param [in] id               SPI controller instance
 * \return SPI FIFO STATUS.
 *
 */
__STATIC_INLINE uint16_t hw_spi_get_fifo_status_reg(HW_SPI_ID id)
{
    return  SBA(id)->SPI_FIFO_STATUS_REG;
}

/**
 *
 * \brief Get SPI transaction status from Status Register
 *
 * \param [in] id               SPI controller instance
 * \return bool.
 *
 */

__STATIC_INLINE bool hw_spi_get_fifo_status_reg_transaction_active(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_FIFO_STATUS_REG, SPI_TRANSACTION_ACTIVE);
}

/**
 *
 * \brief Get SPI TX FIFO level from FIFO Status Register
 *
 * \param [in] id               SPI controller instance
 * \return Number of bytes in TX FIFO.
 *
 */
__STATIC_INLINE uint8_t hw_spi_get_fifo_status_reg_tx_fifo_level(HW_SPI_ID id)
{
        return (uint8_t) HW_SPI_REG_GETF(id, SPI_FIFO_STATUS_REG, SPI_TX_FIFO_LEVEL);
}

/**
 * \brief Get SPI RX FIFO Empty status from FIFO Status Register
 *
 * \param [in] id               SPI controller instance
 *
 * \return SPI RX FIFO empty bit:
 *         0 = RX FIFO is not empty, data can be read,
 *         1 = RX FIFO is empty, data cannot be read
 *
 */
__STATIC_INLINE bool hw_spi_get_fifo_status_reg_rx_empty(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_FIFO_STATUS_REG, SPI_STATUS_RX_EMPTY);
}

/**
 *
 * \brief Get SPI RX FIFO level from FIFO Status Register
 *
 * \param [in] id               SPI controller instance
 * \return Number of bytes in RX FIFO.
 *
 */
__STATIC_INLINE uint8_t hw_spi_get_fifo_status_reg_rx_fifo_level(HW_SPI_ID id)
{
        return (uint8_t) HW_SPI_REG_GETF(id, SPI_FIFO_STATUS_REG, SPI_RX_FIFO_LEVEL);
}

/**
 *
 * \brief Get SPI RX FIFO overflow status from FIFO Status Register
 *
 * \param [in] id               SPI controller instance
 * \return 0 = no overflow. 1 = receive data is not written to FIFO because FIFO was full.
           It clears with SPI_CTRL_REG.SPI_FIFO_RESET.
 *
 */
__STATIC_INLINE uint8_t hw_spi_get_fifo_status_reg_rx_fifo_overflow(HW_SPI_ID id)
{
        return (uint8_t) HW_SPI_REG_GETF(id, SPI_FIFO_STATUS_REG, SPI_RX_FIFO_OVFL);
}


/***************** SPI_FIFO_READ_REG Functions *****************/

/**
 *
 * \brief Read RX FIFO. Read access is permitted only if SPI_RX_FIFO_EMPTY = 0.
 *
 * \param [in] id               SPI controller instance
 * \return 16 LSbits of RX FIFO.
 *
 */
__STATIC_INLINE uint32_t hw_spi_get_fifo_read_reg(HW_SPI_ID id)
{
        return SBA(id)->SPI_FIFO_READ_REG;
}


/***************** SPI_FIFO_WRITE_REG Functions *****************/

/**
 *
 * \brief Write to TX FIFO. Write access is permitted only if SPI_TX_FIFO_FULL is 0.
 *
 * \param [in] id               SPI controller instance
 * \param [in] tx_data          32 bits.
 *
 */
__STATIC_INLINE void hw_spi_set_fifo_write_reg(HW_SPI_ID id, uint32_t tx_data)
{
        SBA(id)->SPI_FIFO_WRITE_REG = tx_data;
}


/***************** SPI_CS_CONFIG_REG Functions *****************/

/**
 *
 * \brief Set CS output in master mode.
 *
 * \param [in] id               SPI controller instance
 * \param [in] cs_mode          CS output in master mode.
 *
 */
__STATIC_INLINE void hw_spi_set_cs_config_reg_mode(HW_SPI_ID id, HW_SPI_CS_MODE cs_mode)
{
        SBA(id)->SPI_CS_CONFIG_REG = cs_mode;
}

/**
 *
 * \brief Get CS output in master mode.
 *
 * \param [in] id               SPI controller instance
 * \return HW_SPI_CS_MODE.
 *
 */
__STATIC_INLINE HW_SPI_CS_MODE hw_spi_get_cs_config_reg_mode(HW_SPI_ID id)
{
    return (HW_SPI_CS_MODE) SBA(id)->SPI_CS_CONFIG_REG;
}


/***************** SPI_TXBUFFER_FORCE_REG Functions *****************/

/**
 *
 * \brief Write SPI_TXBUFFER_FORCE_REG Register
 *
 * \param [in] id               SPI controller instance
 * \param [in] tx_data          Write directly the tx buffer. It must be used only
 *                              in slave mode.
 *
 * \note First write SPI_TXBUFFER_FORCE_REG and then enable chip select (low).
 *       This is mandatory at spi modes 0 and 2.
 *
 * \note SPI3 should be used only as master
 *
 */
__STATIC_INLINE void hw_spi_set_txbuffer_force_reg(HW_SPI_ID id, uint32_t tx_data)
{
#ifdef HW_SPI3
        ASSERT_WARNING(id != HW_SPI3);
#endif
        SBA(id)->SPI_TXBUFFER_FORCE_REG = tx_data;
}


//============== Interrupt handling ============================================
/**
 * \brief Enables the SPI maskable interrupt (MINT) to the CPU
 *
 * \param [in] id SPI controller instance
 *
 */
__STATIC_INLINE void hw_spi_enable_interrupt(HW_SPI_ID id)
{
        if (hw_spi_get_ctrl_reg_tx_en(id) == true) {
                hw_spi_set_irq_mask_reg_tx_empty_en(id, HW_SPI_MINT_ENABLE);
        }
        if (hw_spi_get_ctrl_reg_rx_en(id) == true) {
                hw_spi_set_irq_mask_reg_rx_full_en(id, HW_SPI_MINT_ENABLE);
        }
}

/**
 * \brief Disables the SPI maskable interrupt (MINT) to the CPU
 *
 * \param [in] id SPI controller instance
 *
 */
__STATIC_INLINE void hw_spi_disable_interrupt(HW_SPI_ID id)
{
        hw_spi_set_irq_mask_reg_tx_empty_en(id, HW_SPI_MINT_DISABLE);
        hw_spi_set_irq_mask_reg_rx_full_en(id, HW_SPI_MINT_DISABLE);
}

/**
 * \brief Get the status of the SPI maskable interrupt (MINT) to the CPU
 *
 * \param [in] id SPI controller instance
 *
 * \return SPI maskable interrupt (MINT) status
 *
 */
__STATIC_INLINE HW_SPI_MINT hw_spi_is_interrupt_enabled(HW_SPI_ID id)
{
        return !!(hw_spi_get_irq_mask_reg_tx_empty_en(id) && hw_spi_get_irq_mask_reg_rx_full_en(id));
}


//==================== Configuration functions =================================

/**
 * \brief Switch the SPI module on and off
 *
 * \param [in] id SPI controller instance
 * \param [in] on the SPI module switch:
 *                0 = SPI module switch off (power saving). Everything is reset
 *                    except control registers.
 *                1 = SPI module is in operational mode
 *
 */
__STATIC_INLINE void hw_spi_enable(HW_SPI_ID id, uint8_t on)
{
        if (on) {
                hw_spi_set_ctrl_reg_spi_en(id, true);
        } else {
                hw_spi_set_ctrl_reg_spi_en(id, false);
        }
}

/**
 * \brief Get the on/off status of the SPI module
 *
 * \param [in] id SPI controller instance
 *
 * \return the status of the SPI ON bit:
 *         0 = SPI module switched off,
 *         1 = SPI module switched on
 *
 */
__STATIC_INLINE uint8_t hw_spi_is_enabled(HW_SPI_ID id)
{
        return hw_spi_get_ctrl_reg_spi_en(id);
}

/**
 * \brief Set SPI source clock's divider for the selected SPI clock frequency
 *
 * \note See hw_spi_set_clock_reg_clk_div
 *
 * \param [in] id       SPI controller instance
 * \param [in] freq     selected SPI source clock's divider in master mode
 *
 */
__STATIC_INLINE void hw_spi_set_clock_freq(HW_SPI_ID id, HW_SPI_FREQ freq)
{
        hw_spi_set_clock_reg_clk_div(id, freq);
}

/**
 * \brief Get SPI source clock's divider for the selected SPI clock frequency
 *
 * \note See hw_spi_get_clock_reg_clk_div
 *
 * \param [in] id       SPI controller instance
 *
 * \return SPI source clock's divider in master mode
 *
 */
__STATIC_INLINE HW_SPI_FREQ hw_spi_get_clock_freq(HW_SPI_ID id)
{
        return hw_spi_get_clock_reg_clk_div(id);
}


/**
 * \brief Set SPI master/slave mode
 * \note Configure SPI_CONFIG_REG[SPI_SLAVE_EN] field before SPI enable (SPI_CTRL_REG[SPI_EN] = 1)
 *
 * \param [in] id SPI controller instance
 * \param [in] smn SPI mode - master/slave
 *
 */
__STATIC_INLINE void hw_spi_set_mode(HW_SPI_ID id, HW_SPI_MODE smn)
{
        hw_spi_set_config_reg_slave_en(id, smn);
}

/**
 * \brief Get the SPI master/slave mode
 *
 * \param [in] id SPI controller instance
 *
 * \return SPI master/slave mode
 *
 */
__STATIC_INLINE HW_SPI_MODE hw_spi_is_slave(HW_SPI_ID id)
{
        return hw_spi_get_config_reg_slave_en(id);
}

/**
 * \brief Set SPI word mode
 *
 * \param [in] id SPI controller instance
 * \param [in] word word length
 *
 */
__STATIC_INLINE void hw_spi_set_word_size(HW_SPI_ID id, HW_SPI_WORD word)
{
        hw_spi_set_config_reg_word_len(id, word);
}

/**
 * \brief Get the SPI word mode
 *
 * \param [in] id SPI controller instance
 *
 * \return SPI word mode
 *
 */
__STATIC_INLINE HW_SPI_WORD hw_spi_get_word_size(HW_SPI_ID id)
{
        if (id == HW_SPI1) {
#if HW_SPI1_USE_FIXED_WORD_SIZE == 1
                return (HW_SPI1_FIXED_WORD_SIZE);
#endif
        } else if (id == HW_SPI2) {
#if HW_SPI2_USE_FIXED_WORD_SIZE == 1
                return (HW_SPI2_FIXED_WORD_SIZE);
#endif
        } else if (id == HW_SPI3) {
#if HW_SPI3_USE_FIXED_WORD_SIZE == 1
                return (HW_SPI3_FIXED_WORD_SIZE);
#endif
        } else {
                ASSERT_WARNING(0);
        }
        // Get value of SPI master/slave mode from SPI control register
        return hw_spi_get_config_reg_word_len(id);
}

/**
 * \brief Get the SPI word size
 *
 * Returns number of bytes that will be read/written to/from memory.
 *
 * \param [in] id SPI controller instance
 *
 * \return SPI word mode
 *
 */
__STATIC_INLINE uint32_t hw_spi_get_memory_word_size(HW_SPI_ID id)
{
        // Register Value = 3..31 -> Actual Wordsize = 4..32bits
        // E.g. Register Value = 3..7bits => Actual Wordsize = 4..8bits => Mem_wsz = 1Byte
        uint32_t mem_wsz = (hw_spi_get_word_size(id)>>3) + 1;
        if (mem_wsz == 3) {
                mem_wsz++;
        }
        return mem_wsz;
}

__STATIC_INLINE void hw_spi_reset(HW_SPI_ID id)
{
}

/**
 * \brief Get the value of the SPI TX FIFO full bit
 *
 * \param [in] id SPI controller instance
 *
 * \return SPI TX FIFO full bit:
 *         false = TX FIFO is not full, data can be written,
 *         true = TX FIFO is full, data cannot be written
 *
 */
__STATIC_INLINE bool hw_spi_is_tx_fifo_full(HW_SPI_ID id)
{
        return (bool) HW_SPI_REG_GETF(id, SPI_FIFO_STATUS_REG, SPI_STATUS_TX_FULL);
}



/**
 * \brief Set SPI GPIO Chip Select (CS) Pad
 *
 * \param [in] id SPI controller instance
 * \param [in] cs_pad struct SPI_Pad with GPIO CS port and pin selection
 *
 */
void hw_spi_set_cs_pad(HW_SPI_ID id, const SPI_Pad *cs_pad);

/**
 * \brief Initialize peripheral divider register - select clock source and enable SPI clock
 *
 * \note Function is executed at hw_spi_init. No need to run it when use hw_spi_init interface.
 *
 * \param [in] id SPI controller instance
 * \param [in] select_divn
 *              True = Select DIVN clock source
 *              False = Select DIV1 clock source
 *
 */
void hw_spi_init_clk_reg(const HW_SPI_ID id, bool select_divn);

/**
 * \brief Get the SPI clock source
 *
 * \param [in] id SPI controller instance
 * \return DIVN or DIV1
 *
 * \sa HW_SPI_CLK_SRC
 */
__STATIC_INLINE HW_SPI_CLK_SRC hw_spi_get_clock_source(const HW_SPI_ID id)
{
        if (id == HW_SPI1) {
                return (HW_SPI_CLK_SRC) REG_GETF(CRG_SNC, CLK_SNC_REG, SPI_CLK_SEL);
        }
        if (id == HW_SPI2) {
                return (HW_SPI_CLK_SRC) REG_GETF(CRG_SNC, CLK_SNC_REG, SPI2_CLK_SEL);
        }
        if (id == HW_SPI3) {
                return (HW_SPI_CLK_SRC) REG_GETF(CRG_SYS, CLK_SYS_REG, SPI3_CLK_SEL);
        }
        return HW_SPI_CLK_SRC_INVALID;
}

/**
 * \brief De-initialize peripheral divider register - disable SPI clock
 *
 * \note Function is executed at hw_spi_deinit. No need to run it when use hw_spi_deinit interface.
 *
 * \param [in] id SPI controller instance
 *
 */
void hw_spi_deinit_clk_reg(const HW_SPI_ID id);

/**
 * \brief Initialize the SPI module
 *
 * \note The SPI clock source is set to DIVN (16MHz, regardless of PLL or XTAL16M being used).
 *
 * \param [in] id SPI controller instance
 * \param [in] cfg pointer to SPI configuration struct
 *
 */
void hw_spi_init(HW_SPI_ID id, const spi_config *cfg);

//=========================== CS handling function =============================

/**
 * \brief Set SPI CS low
 *
 * \param [in] id SPI controller instance
 *
 * \sa hw_spi_set_cs_pad, hw_spi_set_cs_config_reg_mode
 *
 */
void hw_spi_set_cs_low(HW_SPI_ID id);

/**
 * \brief Set SPI CS high
 *
 * \param [in] id SPI controller instance
 *
 * \sa hw_spi_set_cs_pad, hw_spi_set_cs_config_reg_mode
 *
 */
void hw_spi_set_cs_high(HW_SPI_ID id);


//=========================== FIFO control functions ===========================

/**
 * \brief Set SPI FIFO mode
 *
 * \param [in] id SPI controller instance
 * \param [in] mode SPI FIFO mode
 *
 */
void hw_spi_set_fifo_mode(HW_SPI_ID id, HW_SPI_FIFO mode);

/**
 * \brief Get SPI FIFO mode
 *
 * \param [in] id SPI controller instance
 *
 * \return currently selected SPI FIFO mode
 *
 */
HW_SPI_FIFO hw_spi_get_fifo_mode(HW_SPI_ID id);

/**
 * \brief Change SPI FIFO mode
 *
 * Unlike hw_spi_set_fifo_mode() it checks current FIFO mode and if mode is going to change
 * waits till all data were sent before changing mode.
 * If mode is same, registers are not touched and no waiting is performed.
 *
 * \param [in] id SPI controller instance
 * \param [in] mode SPI FIFO mode
 *
 * \return previously selected SPI FIFO mode
 *
 *  \sa hw_spi_set_fifo_mode
 *
 */
HW_SPI_FIFO hw_spi_change_fifo_mode(HW_SPI_ID id, HW_SPI_FIFO mode);

#if (HW_SPI_DMA_SUPPORT == 1)
//=========================== DMA control functions ============================

/**
 * \brief Set up both DMA channels and DMA priorities per channel for SPI
 *
 * \param [in] id SPI controller instance
 * \param [in] channel rx channel number, tx channel will be rx + 1
 * \param [in] prio DMA priority per channel
 *
 * \note DMA channel priorities are configured to their default values
 * when prio->use_prio = false or prio == NULL
 *
 * \sa HW_SPI_DMA_TX_PRIO HW_SPI_DMA_TX_PRIO
 */
void hw_spi_configure_dma_channels(HW_SPI_ID id, int8_t channel, const hw_spi_dma_prio_t *prio);

/**
 * \brief Set up both dma channels for SPI
 *
 * \param [in] id SPI controller instance
 * \param [in] channel rx channel number, tx channel will be rx + 1
 * \param [in] pri DMA priority for both channels
 *
 * \deprecated This function is deprecated. User should call hw_spi_configure_dma_channels() instead.
 */
DEPRECATED_MSG("API no longer supported, use hw_spi_configure_dma_channels() instead.")
__STATIC_INLINE void hw_spi_set_dma_channels(HW_SPI_ID id, int8_t channel, HW_DMA_PRIO pri)
{
        const hw_spi_dma_prio_t prio = {
                .use_prio = true,
                .rx_prio = pri,
                .tx_prio = pri,
        };

        hw_spi_configure_dma_channels(id, channel, &prio);
}

#endif /* HW_SPI_DMA_SUPPORT */

//=========================== Other functions ============================

/**
 * \brief Get SPI busy status
 *
 * \param [in] id SPI controller instance
 *
 * \return status of SPI BUSY bit:
 *         0 = The SPI is not busy with a transfer. This means that either
 *             no TX-data is available or that the transfers have been suspended
 *             due to a full RX-FIFO. The SPI interrupt bit can be used to
 *             distinguish between these situations,
 *         1 = The SPI is busy with a transfer
 *
 */
__STATIC_INLINE uint8_t hw_spi_is_busy(HW_SPI_ID id)
{
        // Get the value of the SPI BUSY bit from the secondary SPI control register
        return hw_spi_get_fifo_status_reg_transaction_active(id) ? 1 : 0;
}

/**
 * \brief Wait till SPI is not busy
 *
 * \param [in] id SPI controller instance
 *
 *
 */
__STATIC_INLINE void hw_spi_wait_while_busy(HW_SPI_ID id)
{
        while (hw_spi_is_busy(id));
}

/**
 * \brief Disables SPI controller
 *
 * \param [in] id SPI controller instance
 *
 * \return status of ongoing async transaction
 */
void hw_spi_deinit(HW_SPI_ID id);

/**
 * \brief get SPI transaction status
 *
 * \param [in] id SPI controller instance
 *
 * \return status of ongoing async transaction
 */
bool hw_spi_is_occupied(const HW_SPI_ID id);


#endif /* dg_configUSE_HW_SPI */
#endif /* HW_SPI_H_ */

/**
 * \}
 * \}
 */
