#ifndef CPU_TRAPS_H
#define CPU_TRAPS_H

#include "kernel.h"
#include "vga.h"
#include "idt.h"

struct interrupt_frame {
    uint8_t error;    
};

#define __isr(name) \
    __attribute__((interrupt)) void \
    name(__attribute__((unused)) struct interrupt_frame* frame)

status_t init_cpu_traps(void);

#endif
