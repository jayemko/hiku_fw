/*
 * Copyright (C) 2008-2012, Marvell International Ltd.
 * All Rights Reserved.
 */

/* Entry Point */
ENTRY(Reset_IRQHandler)

/* Start address of main stack pointer */
_estack = 0x20008000;    /* end of 32K  SRAM1 */

/* Heap size in bytes */
_heap_size = (20 * 1024);


MEMORY
{
/*In PM3 only 192K SRAM is alive */
/* 160K of SRAM 0*/
/* 32K of SRAM 1 */
/*  If this demo is to be ued for Z1 with PM3 */
/* Set SRAM0 LENGTH = 64K */

	SRAM0 (rwx)  : ORIGIN = 0x00100000, LENGTH = 160K
	SRAM1 (rwx)  : ORIGIN = 0x20000000, LENGTH = 32K
	NVRAM (rw)   : ORIGIN = 0x480C0000, LENGTH = 4K
}


/* The size of the main stack that is used for interrupt processing. */
_main_stack_size = 0x800;
/* Start of main stack*/
_main_stack_start = ORIGIN(SRAM1) + LENGTH(SRAM1) - _main_stack_size;


SECTIONS
{
	.text :
	{
		. = ALIGN(256);
		KEEP(*(.isr_vector))
		. = ALIGN(4);

		*(.text.Reset_IRQHandler)
		*(.text .text.* .gnu.linkonce.t.*)
		*(.rodata .rodata.* .gnu.linkonce.r.*)

		. = ALIGN(4);
		_etext = .;
	} > SRAM0

	.data :
	{
		_data = .;
		*(.data)
		*(.data.*)
		_edata = .;
	} > SRAM1

	/*
	 * NOTE: Please do not move the below section ".iobufs" to SRAM0.
	 * Some of the peripherals (e.g. USB, SDIO) with their own DMA engines
	 * have a requirement that the data buffers for DMA need to be in the
	 * "Data" memory (SRAM1). The peripherals that use internal DMA engine
	 * of MC200 (e.g. UART) can use data buffers from SRAM0 or SRAM1.
	 */
	.iobufs (NOLOAD):
	{
		. = ALIGN(4);
		_iobufs_start = .;
		*(.iobufs)
		*(.iobufs.*)
		_iobufs_end = .;
	} > SRAM1

	.heapdata (NOLOAD):
	{
		. = ALIGN(4);
		_heap_start = .;
		. += _heap_size;
		. = ALIGN(4);
		_heap_end = .;
	} > SRAM1

	.bss (NOLOAD):
	{
		_bss = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		_ebss = .;
	} > SRAM1

	/* Check for enough space for stack */

        /* Add the free space in SRAM1 to our heap */
	_heap_2_start = .;
	. = _main_stack_start;
	_heap_2_end = .;

        /* The main stack */
        ._main_stack _main_stack_start :
        {
                . = ALIGN(4);
                . = . + _main_stack_size;
                . = ALIGN(4);
        } > SRAM1

	.nvram (NOLOAD):
	{
		/* BootROM uses first few bytes of retention ram */
		_nvram_start = .;
		. = . + 64;
		. = ALIGN(4);
		/* Zero initialized on bootup */
		_nvram_begin = .;
		*(.nvram)
		*(.nvram.*)
		_nvram_end = .;
		/* Un-initialized nvram section */
		. = ALIGN(4);
		*(.nvram_uninit)
		*(.nvram_uninit.*)
	} > NVRAM
}
