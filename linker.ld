/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

 
/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;
 
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.multiboot :
	{
		*(.multiboot.data)
        *(.multiboot.text)
	}
    
    kloadoffset = 0xc0000000;
    . += kloadoffset;

    /* Offset by kloadoffset so that grub still loads kernel at 
     * 1M but the addresses used act as if they are at 3GB */
	.text ALIGN(4K) : AT (ADDR(.text) - kloadoffset)
    {
        *(.text)
    }

	.rodata ALIGN(4K) : AT (ADDR(.rodata) - kloadoffset)
	{
		*(.rodata)
	}
 
	.data ALIGN(4K) : AT (ADDR(.data) - kloadoffset)
	{
		*(.data)
	}
 
	.bss ALIGN(4K) : AT (ADDR(.bss) - kloadoffset)
	{
		*(COMMON)
		*(.bss)
	}

    /DISCARD/ : {
        *(.note.gnu.property)
    }

    . = ALIGN(4K);
    __kernel_end = .;
}
