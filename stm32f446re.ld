/* Entry Point*/
ENTRY(Reset_Handler)

/* Specify memory areas */
MEMORY
{
    flash0 (rx) : org = 0x08000000, len = 256k
    ram0   (wx) : org = 0x20000000, len = 128k    /* SRAM1 + SRAM2 */
    ram1   (wx) : org = 0x20000000, len = 112k    /* SRAM1 */
    ram2   (wx) : org = 0x00000000, len = 16k     /* SRAM2 */
}

_estack = ORIGIN(ram1) + LENGTH(ram1);

/* Define sections */
SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >flash0

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
    } >flash0

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >ram0 AT> flash0

    _sidata = LOADADDR(.data);

    .bss :
    {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)

        _ebss = .;
        __bss_end__ = _ebss;
    } > ram0

} 
