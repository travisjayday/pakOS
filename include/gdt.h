#ifndef GDT_H
#define GDT_H

#include "kernel.h"
#include "virt-mm.h"

/* GDT Entry Access Byte Flags */
#define _GDT_ABPRSNT        1 << 7  // set for being present

#define _GDT_ABPRIVI_R0     0       // set for ring 0 access
#define _GDT_ABPRIVI_R3     3 << 5  // set for ring 3 access

#define _GDT_ABCSDS         1 << 4  // set for code / data segments
#define _GDT_ABSYS          0       // set for system / tss segment

#define _GDT_ABEXEC         1 << 3  // set for executable segment
#define _GDT_ABNEXEC        0       // set for executable segment

#define _GDT_ABDGROWDOWN    1 << 2  // data segment grows down 
#define _GDT_ABDGROWUP      0       // data segment grows up
#define _GDT_ABCEXECLWR     1 << 2  // code is allowed to be executed if ring <= current ring
#define _GDT_ABCEXECEQ      0       // code only executed if ring is equal 

#define _GDT_ABCDRW         1 << 1  // code readable, data writable bit

/* GDT Entry Flag Byte Flags */
#define _GDT_FLG_1BYTG      0       // per byte granularity in `limit'
#define _GDT_FLG_1PG        1 << 3  // per page granularity in `limit'
#define _GDT_FLG_32BIT      1 << 2  // 32 bit protected segment

#define _GDT_AB_DATA0       _GDT_ABPRSNT | _GDT_ABPRIVI_R0 | _GDT_ABCSDS | \
                            _GDT_ABNEXEC | _GDT_ABDGROWUP  | _GDT_ABCDRW
#define _GDT_AB_DATA3       _GDT_ABPRSNT | _GDT_ABPRIVI_R3 | _GDT_ABCSDS | \
                            _GDT_ABNEXEC | _GDT_ABDGROWUP  | _GDT_ABCDRW
#define _GDT_AB_CODE0       _GDT_ABPRSNT | _GDT_ABPRIVI_R0 | _GDT_ABCSDS | \
                            _GDT_ABEXEC  | _GDT_ABCEXECLWR | _GDT_ABCDRW
#define _GDT_AB_CODE3       _GDT_ABPRSNT | _GDT_ABPRIVI_R3 | _GDT_ABCSDS | \
                            _GDT_ABEXEC  | _GDT_ABCEXECLWR | _GDT_ABCDRW
#define _GDT_x86FLAG        _GDT_FLG_1PG | _GDT_FLG_32BIT


struct gdtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed, aligned(8)));

void ___asm_load_gdt(struct gdtr* gdtr);
void ___asm_reload_segments();

status_t init_gdt(void);

#endif
