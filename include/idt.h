#ifndef IDT_H
#define IDT_H

#include "kernel.h"
#include "virt-mm.h"

// see https://wiki.osdev.org/Interrupt_Descriptor_Table
#define _IDT_FLG_TRAPGATE 0x8f
#define _IDT_FLG_INTGATE  0x8e

struct 
idt_entry {
    uint16_t offset_1;      // offset bits 0..15
    uint16_t selector;      // a code segment selector in GDT or LDT
    uint8_t  zero;          // unused, set to 0
    uint8_t  type_attr;     // type and attributes, see below
    uint16_t offset_2;      // offset bits 16..31
};

struct idtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed, aligned(8)));

/*
 *  Registers ISR for the given IRQ as a trap. In the trap ISR, interrupts
 *  are not cleared, so it might cascade. 
 */
void idt_maptrap(uint8_t irq, void* isr);

/*
 * Registers ISR for the given IRQ as interrupt. In the intterupt ISR,
 * interrupts are cleared, so no other interrupts will interrupt. 
 */
void idt_mapint(uint8_t irq, void* isr);

/* Allocates the IDT and loads IDTR into register */
status_t init_idt(void);

#endif
