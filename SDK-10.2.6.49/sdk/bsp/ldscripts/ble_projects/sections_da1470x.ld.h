/*
 * Copyright (C) 2014-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */
/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines the memory regions.
 * It references the following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapBase
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __Vectors_End
 *   __Vectors_Size
 */

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

#define CODE_IS_IN_RAM          (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
#define CODE_IS_IN_FLASH \
        (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) || \
        (dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH)



#if (__RAM9_SIZE_FOR_MAIN_PROC > 0)
#       define RAM9_DATA_FOR_M33_SIZE           (__ram9_area_for_m33_end__ - __ram9_area_for_m33_start__)
#else
#       define RAM9_DATA_FOR_M33_SIZE           0
#endif

#if (__RAM10_SIZE_FOR_MAIN_PROC > 0)
#       define RAM10_DATA_FOR_M33_SIZE          (__ram10_area_for_m33_end__ - __ram10_area_for_m33_start__)
#else
#       define RAM10_DATA_FOR_M33_SIZE          0
#endif

#if CODE_IS_IN_FLASH

#       define INIT                             FLASH
#       define TEXT                             FLASH
#       define DATA                             RAMS
#       define __AT__(x)                        AT (OQSPI_FW_IVT_BASE_ADDRESS + x)
#       define __TEXT_LMA__
#       define __AT_CMAC_MEM__(x)               __AT__(x)

#       define RETENTION_TEXT_SIZE              (__retention_text_end__ - __retention_text_start__)
#       define RETENTION_RAM_INIT_SIZE          (__retention_ram_init_end__ - __retention_ram_init_start__)
#       define NON_RETENTION_RAM_INIT_SIZE      (__non_retention_ram_init_end__ - __non_retention_ram_init_start__)

#elif CODE_IS_IN_RAM

#       define INIT                             RAMC
#       define TEXT                             RAMC
#       define DATA                             RAMS
        /*
         * The following macros are used so that the linker generates
         * an ELF where all code/data load addresses are 0x20010000-based, with
         * a placement that allows for:
         *  - the code will appear at the correct 0-based address (VMA) when
         *    RAM3 is remapped to 0 (i.e. SYS_CTRL_REG[REMAP_ADR0] = 5)
         *  - the data will appear at the proper 0x2005000-based addresses (VMA)
         * With this scheme, everything appears at its proper address, so
         * there is no need  to use the copy table to copy sections to their
         * run-time addresses.
         *
         * This trick is necessary because access through AHB CPUC is limited to
         * RAM0, RAM1, RAM2 and RAM3.
         *
         * Data allocated in RAM9/RAM10 need special handling: they need to go
         * through the copy table mechanism.
         */
#       define __AT__(x)
#       define __TEXT_LMA__                     AT> LMA_RAM
#       define __AT_CMAC_MEM__(x)               AT (ORIGIN(LMA_RAM) + x)

#       define RETENTION_TEXT_SIZE              0
#       define RETENTION_RAM_INIT_SIZE          0
#       define NON_RETENTION_RAM_INIT_SIZE      0

/*
 * The region "LMA_RAM" is used only in RAM builds to set the LMA of text sections
 * is accessible via AHB CPUS.
 */
MEMORY
{
        LMA_RAM                 : ORIGIN = 0x20010000, LENGTH = 1M
}

#endif


#if (dg_configUSE_SEGGER_FLASH_LOADER == 1)

#       define OQSPI_FLASH_ADDRESS              0x38000000
#       define OQSPI_FW_BASE_OFFSET             0x00003000
#       define OQSPI_FW_IVT_OFFSET              0x400
#       define OQSPI_FW_BASE_ADDRESS            (OQSPI_FLASH_ADDRESS + OQSPI_FW_BASE_OFFSET)
#       define OQSPI_FW_IVT_BASE_ADDRESS        (OQSPI_FW_BASE_ADDRESS + OQSPI_FW_IVT_OFFSET)

#else

#       define OQSPI_FW_IVT_BASE_ADDRESS         0x0

#endif /* dg_configUSE_SEGGER_FLASH_LOADER */


ENTRY(Reset_Handler)

SECTIONS
{
        .init_text :
#if (dg_configUSE_SEGGER_FLASH_LOADER == 1)
        AT (OQSPI_FW_IVT_BASE_ADDRESS)
#endif /* dg_configUSE_SEGGER_FLASH_LOADER */
        {
                KEEP(*(.isr_vector))
                /* Interrupt vector remmaping overhead */
                . = 0x200;
                __Vectors_End = .;
                __Vectors_Size = __Vectors_End - __isr_vector;
                *(text_reset*)
                *system_da1470x.c(.text*)

                /*
                 * this is required so that the section that follows will
                 * have a properly aligned LMA in RAM builds
                 */
                . = ALIGN(8);
        } > INIT __TEXT_LMA__

        .text : ALIGN_WITH_INPUT
        {
                . = ALIGN(8);

#if CODE_IS_IN_FLASH
                /* Optimize the code of specific libgcc files by executing them
                 * from the .retention_text section. */
                *(EXCLUDE_FILE(*libnosys.a:sbrk.o
                               *libgcc.a:_aeabi_uldivmod.o
                               *libgcc.a:_muldi3.o
                               *libgcc.a:_dvmd_tls.o
                               *libgcc.a:bpabi.o
                               *libgcc.a:_udivdi3.o
                               *libgcc.a:_clzdi2.o
                               *libgcc.a:_clzsi2.o
#if dgconfigRETAIN_OS_CODE
                               *event_groups.o
                               *list.o
                               *queue.o
                               *tasks.o
                               *timers.o
#endif

#if dg_configRETAIN_GPADC
                               *ad_gpadc.o
                               *hw_gpadc.o
#endif

#if dg_configRETAIN_BSR
                               *sys_bsr.o
                               *hw_bsr.o
#endif
                                ) .text*)
#elif CODE_IS_IN_RAM
                *(.text*)
                *(text_retained)
#endif
                . = ALIGN(4);
                __start_adapter_init_section = .;
                KEEP(*(adapter_init_section))
                __stop_adapter_init_section = .;

                . = ALIGN(4);
                __start_bus_init_section = .;
                KEEP(*(bus_init_section))
                __stop_bus_init_section = .;

                . = ALIGN(4);
                __start_device_init_section = .;
                KEEP(*(device_init_section))
                __stop_device_init_section = .;

                KEEP(*(.init))
                KEEP(*(.fini))

                /* .ctors */
                *crtbegin.o(.ctors)
                *crtbegin?.o(.ctors)
                *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
                *(SORT(.ctors.*))
                *(.ctors)

                /* .dtors */
                *crtbegin.o(.dtors)
                *crtbegin?.o(.dtors)
                *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
                *(SORT(.dtors.*))
                *(.dtors)

                . = ALIGN(4);
                /* preinit data */
                PROVIDE_HIDDEN (__preinit_array_start = .);
                KEEP(*(.preinit_array))
                PROVIDE_HIDDEN (__preinit_array_end = .);

                . = ALIGN(4);
                /* init data */
                PROVIDE_HIDDEN (__init_array_start = .);
                KEEP(*(SORT(.init_array.*)))
                KEEP(*(.init_array))
                PROVIDE_HIDDEN (__init_array_end = .);

                . = ALIGN(4);
                /* finit data */
                PROVIDE_HIDDEN (__fini_array_start = .);
                KEEP(*(SORT(.fini_array.*)))
                KEEP(*(.fini_array))
                PROVIDE_HIDDEN (__fini_array_end = .);

                *(.rodata*)

#if defined(CONFIG_USE_BLE) && CODE_IS_IN_FLASH
                 . = ALIGN(0x100); /* cmi_fw_dst_addr must be 256-byte aligned */
                 cmi_fw_dst_addr = .;

                 __cmi_fw_area_start = .;
                 KEEP(*(.cmi_fw_area*))
                 __cmi_fw_area_end = .;

                 . = ALIGN(0x400); /* .cmac_fw region ends at 1KiB boundary */
                 __cmi_section_end__ = . - 1;
#endif

#if defined(CONFIG_USE_SNC)
                 __snc_fw_area_start = .;
                 KEEP(*(.snc_fw_area*))
                 __snc_fw_area_end = .;
#endif

                KEEP(*(.eh_frame*))
        } > TEXT __TEXT_LMA__

        .ARM.extab :
        {
                *(.ARM.extab* .gnu.linkonce.armextab.*)
        } > TEXT __TEXT_LMA__

        __exidx_start = .;
        .ARM.exidx :
        {
                *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        } > TEXT __TEXT_LMA__
        __exidx_end = .;

        .copy.table :
        {
                . = ALIGN(4);
                __copy_table_start__ = .;

#if CODE_IS_IN_FLASH

                LONG (__etext)
                LONG (__retention_text_start__)
                LONG (RETENTION_TEXT_SIZE)

                LONG (__etext + RETENTION_TEXT_SIZE)
                LONG (__retention_ram_init_start__)
                LONG (RETENTION_RAM_INIT_SIZE)

                LONG (__etext + RETENTION_TEXT_SIZE + RETENTION_RAM_INIT_SIZE)
                LONG (__non_retention_ram_init_start__)
                LONG (NON_RETENTION_RAM_INIT_SIZE)

#endif /* CODE_IS_IN_FLASH */

#if (__RAM9_SIZE_FOR_MAIN_PROC > 0)
                LONG (__etext + RETENTION_TEXT_SIZE + RETENTION_RAM_INIT_SIZE + NON_RETENTION_RAM_INIT_SIZE)
                LONG (__ram9_area_for_m33_start__)
                LONG (RAM9_DATA_FOR_M33_SIZE)
#endif

#if (__RAM10_SIZE_FOR_MAIN_PROC > 0)
                LONG (__etext + RETENTION_TEXT_SIZE + RETENTION_RAM_INIT_SIZE + NON_RETENTION_RAM_INIT_SIZE + RAM9_DATA_FOR_M33_SIZE)
                LONG (__ram10_area_for_m33_start__)
                LONG (RAM10_DATA_FOR_M33_SIZE)
#endif

                __copy_table_end__ = .;
        } > TEXT __TEXT_LMA__

        .zero.table :
        {
                . = ALIGN(4);
                __zero_table_start__ = .;
                LONG (__retention_ram_zi_start__)
                LONG (__retention_ram_zi_end__ - __retention_ram_zi_start__)
                LONG (__shared_section_retained_zi_start__)
                LONG (__shared_section_retained_zi_end__ - __shared_section_retained_zi_start__)
                __zero_table_end__ = .;
        } > TEXT __TEXT_LMA__

        __etext = .;

        /*
        * Retention RAM that should not be initialized during startup.
        * It should be at a fixed RAM address for both the bootloader and the application,
        * so that the bootloader will not alter those data due to conflicts between its
        * .data/.bss sections with application's .retention_ram_uninit section.
        * It is placed in RAM0 with fixed size of dg_configRETAINED_UNINIT_SECTION_SIZE bytes.
        */
        .retention_ram_uninit (NOLOAD) :
        {
                __retention_ram_uninit_start__ = .;

                KEEP(*(nmi_info))
                KEEP(*(hard_fault_info))

                KEEP(*(retention_mem_uninit))

                ASSERT( . <= __retention_ram_uninit_start__ + dg_configRETAINED_UNINIT_SECTION_SIZE,
                        "retention_ram_uninit section overflowed! Increase dg_configRETAINED_UNINIT_SECTION_SIZE.");

                . = __retention_ram_uninit_start__ + dg_configRETAINED_UNINIT_SECTION_SIZE;
                __retention_ram_uninit_end__ = .;
        } > IVT

#if CODE_IS_IN_FLASH
        /*
         * Code in retention RAM
         */
        .retention_text : __AT__ (__etext)
        {
                . = ALIGN(4); /* Required by copy table */
                __retention_text_start__ = .;

                *(text_retained)

                /* Make the '.text' section of specific libgcc files retained, to
                 * optimize perfomance */
                *libnosys.a:sbrk.o (.text*)
                *libgcc.a:_aeabi_uldivmod.o (.text*)
                *libgcc.a:_muldi3.o (.text*)
                *libgcc.a:_dvmd_tls.o (.text*)
                *libgcc.a:bpabi.o (.text*)
                *libgcc.a:_udivdi3.o (.text*)
                *libgcc.a:_clzdi2.o (.text*)
                *libgcc.a:_clzsi2.o (.text*)
#if dgconfigRETAIN_OS_CODE
                *event_groups.o (.text*)
                *list.o (.text*)
                *queue.o (.text*)
                *tasks.o (.text*)
                *timers.o (.text*)
#endif

#if dg_configRETAIN_GPADC
                *ad_gpadc.o (.text*)
                *hw_gpadc.o (.text*)
#endif

#if dg_configRETAIN_BSR
                *sys_bsr.o (.text*)
                *hw_bsr.o (.text*)
#endif

                . = ALIGN(4); /* Required by copy table */
                /* All data end */
                __retention_text_end__ = .;
        } > RAMC

        /*
         * RAMC and the beginning of DATA (i.e. RAMS) are mapped to the same
         * physical RAM area (RAM1/RAM2/RAM3), but through a different address
         * space (via the CPUC and CPUS buses, respectively).
         * Therefore we must create a hole in DATA for the space that is taken up by
         * .retention_text, or else the retained code will be overwritten by
         * the retained data.
         */
        .RAM_UNUSED_THROUGH_CPU_S (NOLOAD) :
        {
                . += RETENTION_TEXT_SIZE;
        } > DATA
#endif

        /*
         * Initialized data in retention RAM
         */
        .retention_ram_init : __AT__ (__etext + RETENTION_TEXT_SIZE)
        {
                . = ALIGN(4); /* Required by copy table */
                __retention_ram_init_start__ = .;

                /*
                 * Retained .data sections that need to be initialized
                 */

                /* Retained data */
                *(privileged_data_init)
                *(.retention)

                *(vtable)

                *(retention_mem_init)
                *(retention_mem_const)

                *libg_nano.a:* (.data*)
                *libnosys.a:* (.data*)
                *libgcc.a:* (.data*)
                *libble_stack_da1470x.a:* (.data*)
                *crtbegin.o (.data*)

                KEEP(*(.jcr*))

#if defined(CONFIG_USE_BLE) && CODE_IS_IN_RAM
                . = ALIGN(0x100); /* cmi_fw_dst_addr must be 256-byte aligned */
                cmi_fw_dst_addr = .;

                __cmi_fw_area_start = .;
                KEEP(*(.cmi_fw_area*))
                __cmi_fw_area_end = .;

                . = ALIGN(0x400); /* .cmac_fw region ends at 1KiB boundary */
                __cmi_section_end__ = . - 1;
#endif

                . = ALIGN(4); /* Required by copy table */
                /* All data end */
                __retention_ram_init_end__ = .;
        } > DATA

        __non_retention_ram_start__ = .;

        /*
         * Initialized RAM area that does not need to be retained during sleep.
         * On RAM projects, they are located in the .retention_ram_init section
         * for better memory handling.
         */
        .non_retention_ram_init : __AT__ (__etext + RETENTION_TEXT_SIZE + RETENTION_RAM_INIT_SIZE)
        {
                . = ALIGN(4); /* Required by copy table */
                __non_retention_ram_init_start__ = .;
                *(EXCLUDE_FILE(*libg_nano.a:* *libnosys.a:* *libgcc.a:* *libble_stack_da1470x.a:* *crtbegin.o) .data*)

                . = ALIGN(4); /* Required by copy table */
                __non_retention_ram_init_end__ = .;
        } > DATA

        __non_retention_ram_end__ = .;

        /*
         * Zero-initialized retention RAM
         */
        .retention_ram_zi (NOLOAD) :
        {
                . = ALIGN(4);
                __retention_ram_zi_start__ = .;

                *(privileged_data_zi)
                *(retention_mem_zi)

                *libg_nano.a:* (.bss*)
                *libnosys.a:* (.bss*)
                *libgcc.a:* (.bss*)
                *libble_stack_da1470x.a:* (.bss*)
                *crtbegin.o (.bss*)

                __HeapBase = .;
                __end__ = .;
                end = __end__;
                KEEP(*(.heap*))
                __HeapLimit = .;

                . = ALIGN(4);
                __retention_ram_zi_end__ = .;
        } > DATA

        .stack_section (NOLOAD) :
        {
                /* Advance the address to avoid output section discarding */
                . = . + 1;
                /* 8-byte alignment guaranteed by vector_table_da1470x.S, also put here for
                 * clarity. */
                . = ALIGN(8);
                __StackLimit = .;
                KEEP(*(.stack*))
                . = ALIGN(8);
                __StackTop = .;
                /* Provide __StackTop to newlib by defining __stack externally. SP will be
                 * set when executing __START. If not provided, SP will be set to the default
                 * newlib nano value (0x80000) */
                PROVIDE(__stack = __StackTop);
        } > DATA

        .shared_section_retained_zi (NOLOAD) :
        {
                __shared_section_retained_zi_start__ = .;
#ifdef CONFIG_USE_SNC
                /* Shared space area defined by SNC must reside in the first part of the
                 * SHARED RAM region. */
                KEEP(*(.snc_shared_ram_area*))
                . = ALIGN(4);
#endif
                *(retention_mem_shared_zi)
                __shared_section_retained_zi_end__ = .;
        } > SHARED

#if (dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM == 1)
        /*
         * External RAM section that is not initialized.
         * It is located in PSRAM and it is defined by dg_configEXTERNAL_RAM_BASE and
         * dg_configEXTERNAL_RAM_SIZE
         */
        .external_ram_uninit (NOLOAD) :
        {
                __external_ram_uninit_start__ = .;

                KEEP(*(external_mem_uninit))

                __external_ram_uninit_end__ = .;
        } > EXTERNAL_RAM
#endif

#if (__RAM9_SIZE_FOR_MAIN_PROC > 0)
        /*
         * Initialized data of the MAIN processor to be allocated in RAM9
         */
        .ram9_area_for_m33 :
                __AT_CMAC_MEM__ (__etext + RETENTION_TEXT_SIZE + RETENTION_RAM_INIT_SIZE + NON_RETENTION_RAM_INIT_SIZE)
        {
                __ram9_area_for_m33_start__ = .;

                KEEP(*(m33_data_in_ram9))

                __ram9_area_for_m33_end__ = .;
        } > CMAC2

        /*
         * Uninitialized data of the MAIN processor to be allocated in RAM9
         */
        .ram9_uninit_area_for_m33 (NOLOAD) :
        {
                KEEP(*(m33_uninit_data_in_ram9))
        } > CMAC2
#endif

#if (__RAM10_SIZE_FOR_MAIN_PROC > 0)
        /*
         * Initialized data of the MAIN processor to be allocated in RAM10
         */
        .ram10_area_for_m33 :
                __AT_CMAC_MEM__ (__etext + RETENTION_TEXT_SIZE + RETENTION_RAM_INIT_SIZE + NON_RETENTION_RAM_INIT_SIZE + RAM9_DATA_FOR_M33_SIZE)
        {
                __ram10_area_for_m33_start__ = .;

                KEEP(*(m33_data_in_ram10))

                __ram10_area_for_m33_end__ = .;
        } > CMAC

        /*
         * Uninitialized data of the MAIN processor to be allocated in RAM10
         */
        .ram10_uninit_area_for_m33 (NOLOAD) :
        {
                KEEP(*(m33_uninit_data_in_ram10))
        } > CMAC
#endif

        /*
         * Note that region [__bss_start__, __bss_end__] will be also zeroed by newlib nano,
         * during execution of __START.
         */
        .bss :
        {
                . = ALIGN(4);
                __bss_start__ = .;

                *(EXCLUDE_FILE(*libg_nano.a:* *libnosys.a:* *libgcc.a:* *libble_stack_da1470x.a:* *crtbegin.o) .bss*)

                *(COMMON)
                . = ALIGN(4);
                __bss_end__ = .;
        } > DATA

#if CODE_IS_IN_FLASH
        __FLASH_BUILD__ = .;
#endif
#if CODE_IS_IN_RAM
        __RAM_BUILD__ = .;
#endif

        __unused_ram_start__ = . + 1;

#define IMAGE_SIZE \
        (__etext + RETENTION_TEXT_SIZE + RETENTION_RAM_INIT_SIZE + NON_RETENTION_RAM_INIT_SIZE \
                + RAM9_DATA_FOR_M33_SIZE + RAM10_DATA_FOR_M33_SIZE)

#if (dg_configUSE_SEGGER_FLASH_LOADER == 1)
        .prod_head :
        AT (OQSPI_FLASH_ADDRESS)

        SUBALIGN(1)
        {
                SHORT(0x7050)                   // 'Pp' flag
                LONG(OQSPI_FW_BASE_OFFSET)      // active image pointer
                LONG(OQSPI_FW_BASE_OFFSET)      // update image pointer
                KEEP(*(__product_header_primary__))
                . = 0x1000;
        } > TEXT =0xFFFFFFFF

        .prod_head_backup :
        AT (OQSPI_FLASH_ADDRESS + 0x1000)

        SUBALIGN(1)
        {
                SHORT(0x7050)                   // 'Pp' flag
                LONG(OQSPI_FW_BASE_OFFSET)      // active image pointer
                LONG(OQSPI_FW_BASE_OFFSET)      // update image pointer
                KEEP(*(__product_header_backup__))
                . = (OQSPI_FW_BASE_OFFSET - 0x1000);
        }> TEXT =0xFFFFFFFF

        .img_head :
        AT (OQSPI_FW_BASE_ADDRESS)
        {
                SHORT(0x7151)                   // 'Pp' flag
                LONG(OQSPI_FW_BASE_OFFSET + OQSPI_FW_IVT_OFFSET + IMAGE_SIZE)
                LONG(0x00000000)                // crc, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // timestamp, doesn't matter
                LONG(OQSPI_FW_IVT_OFFSET)       // IVT pointer
                SHORT(0x22AA)                   // Security section type
                SHORT(0x0000)                   // Security section length
                SHORT(0x44AA)                   // Device admin type
                SHORT(0x0000)                   // Device admin length
                . = OQSPI_FW_IVT_OFFSET;
        } > TEXT =0xFFFFFFFF
#endif /* dg_configUSE_SEGGER_FLASH_LOADER */

#if CODE_IS_IN_FLASH
        /* Make sure that the initialized data fits in flash */
        ASSERT(IMAGE_SIZE < ORIGIN(TEXT) + dg_configQSPI_MAX_IMAGE_SIZE, "TEXT space overflowed")
#endif
}

