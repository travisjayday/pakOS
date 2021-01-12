#include "idt.h"

struct idt_entry* idt_base = 0;
struct idtr __idtr; 

void 
___asm_load_idt(struct idtr* idtr) 
{
    asm inline("lidt (%0)"
        : /* No outputs */
        : "r" (idtr));
}

struct idt_entry* 
__make_idt_desc(void* isr, uint8_t type) 
{
    struct idt_entry* entry = (struct idt_entry*) kmalloc(sizeof(struct idt_entry));
    uint32_t offset = (uint32_t) isr;
    entry->offset_1 = (uint16_t) offset;
    entry->selector = 0x8;  // select entry 1 in GDT (code0)
    entry->zero = 0;
    entry->type_attr = type;
    entry->offset_2 = (uint16_t) (offset >> 16);
    return entry;
}

void 
idt_mapint(uint8_t irq, void* isr) 
{
    idt_base[irq] = *__make_idt_desc(isr, _IDT_FLG_INTGATE);
}

void 
idt_maptrap(uint8_t irq, void* isr) 
{
    idt_base[irq] = *__make_idt_desc(isr, _IDT_FLG_TRAPGATE);
}

status_t
init_idt()
{
    uint8_t num_entries = 255;
    idt_base = kmalloc(sizeof(struct idt_entry) * num_entries); 

    //idt_base[0] = __make_idt_desc(0, );

    __idtr.base = (uintptr_t) idt_base;
    __idtr.limit = sizeof(struct idt_entry) * num_entries;
    ___asm_load_idt(&__idtr);

    return STATUS_OK;
}
