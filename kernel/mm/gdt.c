#include "gdt.h"

uint64_t* gdt_base = 0;
struct gdtr __gdtr; 

// see https://wiki.osdev.org/LGDT
uint64_t 
__make_gdt_desc(uint32_t base, uint32_t limit, uint8_t access, uint8_t flag) 
{
    // why... just why...
    uint64_t desc = 0; 

    // let's work on the upper 32 bits first (32-63)
    desc |= (base & 0xff0000) >> 16;    // set bits 32-39
    desc |= access << 8;                // set bits 40-47 
    desc |= limit & 0x000f0000;         // set bits 51-58
    desc |= (flag & 0xf) << 20;         // set bits 52-55
    desc |= (base & 0xff000000);        // set bits 56-63

    // now let's work on the lower 32 bits
    desc <<= 32;
    desc |= limit & 0x0000ffff;         // set bits 0-15
    desc |= (base & 0x0000ffff) << 16;  // set bits 16-31

    return desc;
}

void 
___asm_load_gdt(struct gdtr* gdtr) 
{
    asm inline("lgdt (%0)"
        : /* No outputs */
        : "r" (gdtr));
}

void
___asm_reload_segments() 
{
    asm inline ("jmp    $0x8, $label%=\n\t"
    "label%=:                        \n\t"
        "mov     %0, %%ds       \n\t"
        "mov     %0, %%es       \n\t"
        "mov     %0, %%fs       \n\t"
        "mov     %0, %%gs       \n\t"
        "mov     %0, %%ss       \n\t"
    : /* No outputs */
    : "r" (0x10)); 
}
    
status_t
init_gdt(void)
{
    uint8_t num_entries = 5;
    gdt_base = kmalloc(8 * num_entries); 

    gdt_base[0] = __make_gdt_desc(0, 0, 0, 0);
    gdt_base[1] = __make_gdt_desc(0x0, 0xffffffff, _GDT_AB_CODE0, _GDT_x86FLAG);
    gdt_base[2] = __make_gdt_desc(0x0, 0xffffffff, _GDT_AB_DATA0, _GDT_x86FLAG);
    gdt_base[3] = __make_gdt_desc(0x0, 0xffffffff, _GDT_AB_CODE3, _GDT_x86FLAG);
    gdt_base[4] = __make_gdt_desc(0x0, 0xffffffff, _GDT_AB_DATA3, _GDT_x86FLAG);

    __gdtr.base = (uintptr_t) gdt_base;
    __gdtr.limit = 8 * num_entries;
    ___asm_load_gdt(&__gdtr);

    /* Pre-baked linear GDT values from OSDev. 
    gdt_base[0] = 0;
    gdt_base[1] = 0x00CF9A000000FFFF;
    gdt_base[2] = 0x00CF92000000FFFF;
    gdt_base[3] = 0x00CFFA000000FFFF; 
    gdt_base[4] = 0x00CFF2000000FFFF;
    */

    ___asm_reload_segments();

    return STATUS_OK;
}
