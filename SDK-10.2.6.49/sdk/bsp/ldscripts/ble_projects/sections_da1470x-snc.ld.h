/*
 * Copyright (C) 2014-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ROM and RAM.
 * It references following symbols, which must be defined in code:
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
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

ENTRY(Reset_Handler)

SECTIONS
{
        .text :
        {
                KEEP(*(.isr_vector))
                __SNC_sleep_status = .;
                LONG(0x0)
                *(text_reset*)
                *system_da1470x-snc.c(.text*)
                *(.text*)

                __start_adapter_init_section = .;
                KEEP(*(adapter_init_section))
                __stop_adapter_init_section = .;

                __start_bus_init_section = .;
                KEEP(*(bus_init_section))
                __stop_bus_init_section = .;

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

                KEEP(*(.eh_frame*))
        } > SNC

        .ARM.extab :
        {
                *(.ARM.extab* .gnu.linkonce.armextab.*)
        } > SNC

        __exidx_start = .;
        .ARM.exidx :
        {
                *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        } > SNC
        __exidx_end = .;

        .copy.table :
        {
                . = ALIGN(4);
                __copy_table_start__ = .;
                LONG (__etext)
                LONG (__data_start__)
                LONG (__data_end__ - __data_start__)
                __copy_table_end__ = .;
        } > SNC

        .zero.table :
        {
                . = ALIGN(4);
                __zero_table_start__ = .;
                LONG (__retention_ram_zi_start__)
                LONG (__retention_ram_zi_end__ - __retention_ram_zi_start__)
                /* Add each additional bss section here */
                __zero_table_end__ = .;
        } > SNC

        /*
         * Location counter can end up 2byte aligned with narrow Thumb code but
         * __etext is assumed by startup code to be the LMA of a section in RAM
         * which must be 4byte aligned
         */
        __etext = ALIGN (4);

        .data : AT (__etext)
        {
                __data_start__ = .;
                *(vtable)
                *(.data)
                *(.data.*)

                *(privileged_data_init)
                *(.retention)

                *(retention_mem_init)
                *(retention_mem_const)

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

                KEEP(*(.jcr*))
                . = ALIGN(4);
                /* All data end */
                __data_end__ = .;
        } > SNC

        /*
         * Zero-initialized retention RAM
         */
        .retention_ram_zi (NOLOAD) :
        {
                . = ALIGN(4);
                __retention_ram_zi_start__ = .;
                *(privileged_data_zi)
                *(retention_mem_zi)
                . = ALIGN(4);
                __retention_ram_zi_end__ = .;
        } > SNC

        /*
         * Retention RAM that should not be initialized during startup.
         */
        .retention_ram_uninit (NOLOAD) :
        {
                . = ALIGN(4);
                __retention_ram_uninit_start__ = .;
                KEEP(*(retention_mem_uninit))
                __retention_ram_uninit_end__ = .;
        } > SNC

        /*
         * NOLOAD is used to avoid the "section `.bss' type
         * changed to PROGBITS" warning
         */
        .bss (NOLOAD) :
        {
                . = ALIGN(4);
                __bss_start__ = .;
                *(.bss)
                *(.bss.*)
                *(COMMON)
                . = ALIGN(4);
                __bss_end__ = .;
        } > SNC

        .heap (NOLOAD) :
        {
                . = ALIGN(8);
                __end__ = .;
                PROVIDE(end = .);
                KEEP(*(.heap*))
                . = ALIGN(8);
                __HeapLimit = .;
        } > SNC

        .stack (ORIGIN(SNC) + LENGTH(SNC) - __STACK_SIZE) (COPY) :
        {
                . = ALIGN(8);
                __StackLimit = .;
                KEEP(*(.stack*))
                . = ALIGN(8);
                __StackTop = .;
        } > SNC
        PROVIDE(__stack = __StackTop);

        /* Check if data + heap + stack exceeds RAM limit */
        ASSERT(__StackLimit >= __HeapLimit, "region SNC overflowed with stack")

        /*
         * Shared memory region (SNC RW / CM33 RW)
         */

        /* Allocate some memory for shared SNC-CM33 space */
        .snc_shared (NOLOAD) :
        {
                __snc_shared_start__ = .;

                . = ALIGN(4);
                __snc_shared_space_start__ = .;
                KEEP(*(retention_mem_shared_zi))
                __snc_shared_space_end__ = .;

                . = ALIGN(4);
                __snc_exception_space_start__ = .;
                KEEP(*(nmi_info))
                KEEP(*(hard_fault_info))
                __snc_exception_space_end__ = .;

                . = ALIGN(4);
                __snc_shared_end__ = .;
        } > SHARED
}
