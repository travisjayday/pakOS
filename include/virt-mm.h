#ifndef VIRT_MM_H
#define VIRT_MM_H

#include <stdint.h>
#include <stddef.h>

#include "phy-mm.h"
#include "kernel.h"
#include "kmalloc.h"

#define newl vga_print("\r\n")

/* 
 * Macro to convert from logical to physical address by doing a TLB lookup 
 * mapfr and umapfr do this step by step, but here's a macro for convenience 
 */

#define _lpa(lgaddr)                                        \
    *((uint32_t*)                                           \
            (__logical_pde_base | (((uint32_t) lgaddr >> 22) << 12))   \
            + (((uint32_t) lgaddr >> 12) & 0x3ff)                      \
     ) & ~0x3ff


/* Logical address of PDE table. PDE table lives in higher kernel space
 * and is statically allocated in boot.s */

extern uint32_t pde_table[];

/* Maps a frame (hardware address) to logical address */

void mapfr(void* hwaddr, void* lgaddr);

/* Unamps a logical address from a hardware frame */

void umapfr(void* lgaddr);

/* Initializes the kernel heap */

status_t init_virtmm(void);

/* Allocates and maps length bytes to requested logical address */

status_t mmap(void* addr, size_t length);
status_t munmap(void* addr, size_t length);

/* 
 * Given an address to a memory mapped region in physical RAM, 
 * map that region into logical kernel space and return a pointer
 * to it.
 */
void* maphwdev(uint32_t hwaddr, size_t pages);

/* 
 * Invalidates a single page by clearing MMU cache. Faster than resetting CR3.
 * Inline assembly for GCC (from Linux kernel source) OwO
 */
static inline void __native_flush_tlb_single(unsigned long addr)
{
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

static inline void __flush_all_tlb() {
    __asm__("movl %cr3, %ecx\n\t"
            "movl %ecx, %cr3\n\t");
}


#endif
