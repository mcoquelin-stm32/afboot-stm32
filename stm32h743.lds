ENTRY(reset)
MEMORY
{
	FLASH (RX)  : ORIGIN = 0x08000000, LENGTH = 0x00200000
	SRAM1 (RWX) : ORIGIN = 0x24000000, LENGTH = 176 * 1024 /*0x00040000*/
	STACK (RW)  : ORIGIN = 0x24000000 + 176 * 1024 - 1, LENGTH = 0
}
SECTIONS
{
	. = ORIGIN(FLASH);

	.text :
	{
		KEEP(*(.vector_table))
		*(.text)
		*(.rodata)
		*(.rodata.*)
		/*. = ALIGN(4);*/
		_end_text = .;
	} >FLASH

	.data :
	{
		_start_data = .;
		*(.data)
		_end_data = .;
	} >SRAM1 AT >FLASH

	.bss :
	{
		_start_bss = .;
		*(.bss)
		_end_bss = .;
	} >SRAM1

	.stack :
	{
		_stack_top = .;
	} >STACK
}
