/**
 * \addtogroup MIDDLEWARE_RPMSG_LITE
 * \{
 *
 * \addtogroup MIDDLEWARE_RPMSG_PLATFORM Platform
 * \brief SDK Platform Layer
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file rpmsg_platform.h
 *
 * @brief RPMsg-Lite DA1470x platform layer header file
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef RPMSG_PLATFORM_H_
#define RPMSG_PLATFORM_H_

#include <stdint.h>

/*
 * No need to align the VRING as defined in Linux because DA1470x is not intended
 * to run the Linux
 */
#ifndef VRING_ALIGN
#define VRING_ALIGN (0x10U)
#endif

/* contains pool of descriptos and two circular buffers */
#ifndef VRING_SIZE
#define VRING_SIZE (0x400UL)
#endif

/* size of shared memory + 2*VRING size */
#define RL_VRING_OVERHEAD (2UL * VRING_SIZE)

#define RL_GET_VQ_ID(link_id, queue_id) (((queue_id)&0x1U) | (((link_id) << 1U) & 0xFFFFFFFEU))
#define RL_GET_LINK_ID(id)              (((id)&0xFFFFFFFEU) >> 1U)
#define RL_GET_Q_ID(id)                 ((id)&0x1U)

#define RL_PLATFORM_DA1470X_M33_SNC_LINK_ID  (0U)
#define RL_PLATFORM_HIGHEST_LINK_ID          (0U)

/*
 * Allocate space for RPMsg-Lite data in shared memory.
 * Real needs in memory must be defined per application.
 */
#ifndef RL_PLATFORM_SH_MEM_SIZE
#define RL_PLATFORM_SH_MEM_SIZE              (6144U)
#endif

/* platform interrupt related functions */
int32_t platform_init_interrupt(uint32_t vector_id, void *isr_data);
int32_t platform_deinit_interrupt(uint32_t vector_id);
int32_t platform_interrupt_enable(uint32_t vector_id);
int32_t platform_interrupt_disable(uint32_t vector_id);
int32_t platform_in_isr(void);
void platform_notify(uint32_t vector_id);

/* platform low-level time-delay (busy loop) */
void platform_time_delay(uint32_t num_msec);

/* platform memory functions */
void platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr, uint32_t size, uint32_t flags);
void platform_cache_all_flush_invalidate(void);
void platform_cache_disable(void);
uint32_t platform_vatopa(void *addr);
void *platform_patova(uint32_t addr);

/* platform init/deinit */
int32_t platform_init(void);
int32_t platform_deinit(void);

/**
 * \brief Get the RPMsg-Lite base address in the shared memory
 *
 * \return the RPMsg-Lite base address in the shared memory
 */
void *platform_get_base_addr(void);

/**
 * \}
 *
 * \}
 */

#endif /* RPMSG_PLATFORM_H_ */
