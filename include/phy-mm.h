#ifndef PHYMM_H
#define PHYMM_H

#include "vga.h"
#include "kernel.h"

#define newl vga_print("\r\n")

#define _PGUSE 1
#define _PGFREE 0

#define mempanic() \
    newl; newl;\
    vga_print("Memory panic");\
    __asm__("jmp .");

/*
 * Symbols for handling hardware frame allocations. We are using a stack model
 * to keep track of available hardware page frames. The stack starts after kernel
 * data in high memory and is initially filled with available page frames extracted
 * from multiboot memory info. Note that the stack may grow at most 4M if we have
 * 4GB of available RAM. 
 */

/* Base of stack starts after kernel */

// declared in linker.ld
extern uint8_t __kernel_end[];

/* _hwas_bp == __kernel_end */
extern uint32_t* _hwas_bp;
extern uint32_t* _hwas_sp;
extern uint32_t _hwas_ssize;

/* hw address of where physical pages start coming from. Valid pages can be
 * drawn from _hwa_mmstart to _hwa_mmstart + _hwas_ssize */

extern uint8_t* _hwa_mmstart;

/* bitmap to accompany the stack. Note this does not index all ram pages, 
 * but only those that we usen in _hwas_stack. Allocated above stack in ram. */
extern uint8_t* _hwa_inu_bitmap; 
#define _hwa_inu_bitmap_size _hwas_ssize / 8

/* page aligned end of logical memory used by this physical memory manager */
extern uint8_t* __phymm_end;

/* 
 * Return whether the page at `hwaddr' is in use or not, according to the
 * _hwa_inu_bitmap. 
 */
#define _hwa_inubm_get(hwaddr) ({                                       \
    uint32_t pgidx = ((uint8_t*)hwaddr - _hwa_mmstart) / 4096;                    \
    (_hwa_inu_bitmap[pgidx / 8] & (1 << (pgidx % 8))) >> (pgidx % 8);   \
})

/*
 * Sets the page at `hwaddr' as in use or not in the _hwa_inu_bitmap.
 */ 
#define _hwa_inubm_set(hwaddr, in_use) ({                                \
    uint32_t pgidx = ((uint8_t*)hwaddr - _hwa_mmstart) / 4096;                     \
    if (in_use != 0) (_hwa_inu_bitmap[pgidx / 8] |= (1 << (pgidx % 8))); \
    else             (_hwa_inu_bitmap[pgidx / 8] &= ~(1 << (pgidx % 8)));\
})

struct bios_mmap_entry {
    uint32_t entry_size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type; 
} __attribute__((packed));

extern uint32_t bios_mmap_len;
extern struct bios_map_entry* bios_mmap_addr;

/* 
 * Returns an unused page hw frame from the stack. If n > 1, returns 
 * address that holds n consecutive hardware pages and marks them as 
 * HWF_INUSE (hardware frame in use) in the stack. 
 */

void* hwpalloc(uint16_t n);
status_t hwpfree(void* hwaddr, uint16_t n);
status_t init_phymm(void);

#endif
