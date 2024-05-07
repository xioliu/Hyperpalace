ENTRY(_entry)

STACKTOP = 0X41000000;

SECTIONS
{
    . = 0X40000000;
    __start = .;

    __text_start = .;
    .text :
    {
    KEEP(*(.boot))
    *(.text)
    }
    . = ALIGN(4096);
    __text_end = .;

    __data_start = .;
    .data :
    {
    *(.data)
    }
    . = ALIGN(4096);
    __data_end = .;

    __bss_start = .;
    .bss :
    {
    bss = .;
    *(.bss)
    }
    . = ALIGN(4096);
    __bss_end = .;
    __end = .;

    . = STACKTOP;
    stack_top = .;
}