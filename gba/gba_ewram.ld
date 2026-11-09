/* SDLPoP for GBA: relocates the giant SDLPoP .data/.bss out of the 32 KB
   IWRAM into the 256 KB EWRAM. Based on devkitARM's stock gba_cart.ld v1.5.
   Sections kept in IWRAM: .iwram (explicit), small overlays, stack/IRQ. */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
	rom	: ORIGIN = 0x08000000, LENGTH = 32M
	iwram	: ORIGIN = 0x03000000, LENGTH = 32K
	ewram	: ORIGIN = 0x02000000, LENGTH = 256K
}

__text_start	= ORIGIN(rom);
__eheap_end	= ORIGIN(ewram) + LENGTH(ewram);
__iwram_start	= ORIGIN(iwram);
__iwram_top	= ORIGIN(iwram) + LENGTH(iwram);
__sp_irq	= __iwram_top - 0x060;
__sp_usr	= __sp_irq - 0x0a0;
__irq_flags	= 0x03007ff8;

SECTIONS
{
	. = __text_start;

	.crt0 : { KEEP (*(.crt0)) . = ALIGN(4); } >rom = 0xff
	.init : { KEEP (*(SORT_NONE(.init))) } >rom
	.plt  : { *(.plt) . = ALIGN(4); } >rom

	.text  :
	{
		*(EXCLUDE_FILE (*.iwram*) .text*)
		*(.gnu.linkonce.t.*)
		KEEP (*(.text.*personality*))
		*(.gnu.warning)
		*(.glue_7t) *(.glue_7) *(.vfp11_veneer)
		. = ALIGN(4);
	} >rom = 0xff

	__text_end = .;

	.fini : { KEEP (*(.fini)) . = ALIGN(4); } >rom = 0

	.rodata :
	{
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);
	} >rom = 0xff

	.ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >rom
	__exidx_start = .;
	.ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >rom
	__exidx_end = .;

	.ctors :
	{
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		. = ALIGN(4);
	} >rom = 0

	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		. = ALIGN(4);
	} >rom = 0

	.eh_frame         : { KEEP (*(.eh_frame)) . = ALIGN(4); } >rom = 0
	.gcc_except_table : { *(.gcc_except_table) . = ALIGN(4); } >rom = 0

	__iwram_lma = .;

	.iwram __iwram_start : AT (__iwram_lma)
	{
		__iwram_start__ = ABSOLUTE(.);
		*(.iwram .iwram*)
		*iwram.*(.text* .data*)
		. = ALIGN(4);
		__iwram_end__ = ABSOLUTE(.);
	} >iwram = 0xff

	__data_lma = __iwram_lma + SIZEOF(.iwram);
	__iwram_overlay_lma = __data_lma;
	__iwram_overlay_start = .;
	__iwram_overlay_end = .;
	__load_start_iwram0 = __iwram_overlay_start;
	__load_stop_iwram0  = __iwram_overlay_end;
	__load_start_iwram1 = __iwram_overlay_start;
	__load_stop_iwram1  = __iwram_overlay_end;
	__load_start_iwram2 = __iwram_overlay_start;
	__load_stop_iwram2  = __iwram_overlay_end;
	__load_start_iwram3 = __iwram_overlay_start;
	__load_stop_iwram3  = __iwram_overlay_end;
	__load_start_iwram4 = __iwram_overlay_start;
	__load_stop_iwram4  = __iwram_overlay_end;
	__load_start_iwram5 = __iwram_overlay_start;
	__load_stop_iwram5  = __iwram_overlay_end;
	__load_start_iwram6 = __iwram_overlay_start;
	__load_stop_iwram6  = __iwram_overlay_end;
	__load_start_iwram7 = __iwram_overlay_start;
	__load_stop_iwram7  = __iwram_overlay_end;
	__load_start_iwram8 = __iwram_overlay_start;
	__load_stop_iwram8  = __iwram_overlay_end;
	__load_start_iwram9 = __iwram_overlay_start;
	__load_stop_iwram9  = __iwram_overlay_end;

	. = ORIGIN(ewram);

	/* .bss in EWRAM */
	.bss ALIGN(4) (NOLOAD) :
	{
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = ABSOLUTE(.);
	} >ewram

	/* .data in EWRAM. LMA in ROM so crt0 can copy it in. */
	.data ALIGN(4) : AT (__data_lma)
	{
		__data_start__ = ABSOLUTE(.);
		*(.data*)
		*(.gnu.linkonce.d*)
		CONSTRUCTORS
		. = ALIGN(4);
	} >ewram = 0xff

	__preinit_lma = __data_lma + SIZEOF(.data);

	.preinit_array ALIGN(4) : AT (__preinit_lma)
	{
		__preinit_array_start = ABSOLUTE(.);
		KEEP (*(.preinit_array))
		__preinit_array_end = ABSOLUTE(.);
	} >ewram

	__init_lma = __preinit_lma + SIZEOF(.preinit_array);

	.init_array ALIGN(4) : AT (__init_lma)
	{
		__init_array_start = ABSOLUTE(.);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = ABSOLUTE(.);
	} >ewram

	__fini_lma = __init_lma + SIZEOF(.init_array);

	.fini_array ALIGN(4) : AT (__fini_lma)
	{
		__fini_array_start = ABSOLUTE(.);
		KEEP (*(SORT(.fini_array.*)))
		KEEP (*(.fini_array))
		__fini_array_end = ABSOLUTE(.);
	} >ewram

	__jcr_lma = __fini_lma + SIZEOF(.fini_array);
	.jcr ALIGN(4) : AT (__jcr_lma) { KEEP (*(.jcr)) } >ewram

	__data_end__ = ABSOLUTE(.);

	/* The 'ewram' section the upstream crt0 expects. Place after .data.
	   __ewram_start must be defined here, NOT at ORIGIN(ewram): crt0 uses
	   __ewram_end - __ewram_start as the byte count for an LMA->VMA copy,
	   so starting it at 0x02000000 would memcpy ~96 KB of garbage from
	   past the end of the ROM over .bss / .data and prevent boot. */
	. = ALIGN(4);
	__ewram_lma = __jcr_lma + SIZEOF(.jcr);
	__ewram_start = .;
	.ewram : AT (__ewram_lma)
	{
		*(.ewram*)
		. = ALIGN(4);
		__ewram_end = ABSOLUTE(.);
	} >ewram = 0xff

	__pad_lma = __ewram_lma + SIZEOF(.ewram);

	.sbss ALIGN(4) (NOLOAD) :
	{
		__sbss_start__ = ABSOLUTE(.);
		*(.sbss*)
		. = ALIGN(4);
		__sbss_end__   = ABSOLUTE(.);
		__end__        = ABSOLUTE(.);
		__eheap_start  = ABSOLUTE(.);
		__iheap_start  = ABSOLUTE(__iwram_end__);
	} >ewram

	.pad ALIGN(4) : AT (__pad_lma)
	{
		LONG(0x52416b64)
		LONG(0x4d)
		. = ALIGN(4);
	} = 0xff
	__rom_end__ = __pad_lma + SIZEOF(.pad);

	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.stack 0x80000   : { _stack = .; *(.stack) }
	.note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
	.ARM.attributes     0 : { KEEP (*(.ARM.attributes)) }
	/DISCARD/ : { *(.note.GNU-stack) }
}
