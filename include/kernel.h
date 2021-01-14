#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include "vga.h"

#define status_t uint8_t
#define STATUS_OK           0
#define STATUS_BORKD_MEM    0xFF
#define STATUS_DOUBLE_FREE  0xFE
#define STATUS_INVALID_MEM  0xFD
#define STATUS_FAILED_INIT  0xFC
#define STATUS_OUT_OF_MEM   0xFB
#define STATUS_INVALID_FREE 0xFA

#define DEBUG 5

#define KBREAK __asm__("jmp .")
#define KPANIC vga_print("Kernel Panic"); KBREAK;                  

/* 
 * The location of where our recursive PDE table can be accessed 
 * Since we map the 1023rd index to the PDE base, we can access
 * it at the following address
 */
static const uint32_t __logical_pde_base    = 0xffc00000;

/* 
 * Logical address where the kernel heap starts.
 * Note: heap grows upward. 
 */
static const uint32_t __kheap_start         = 0xd0000000;

/* 
 * Kernel heap size constraints.
 */
static const uint32_t __kheap_min_size      = 4096 * 1;      // intial size
static const uint32_t __kheap_max_size      = 0x0fffffff;     // max size

void _kguard (status_t status);

/*
 * Wrappers for outb and inb asm instructions.
 */
static inline void 
___outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void 
___outl(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t 
___inb(uint16_t port) {
    uint8_t ret; asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline uint32_t 
___inl(uint16_t port) {
    uint32_t ret; asm volatile ( "inl %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}


/*
 * Keeps CPU busy for a little bit so that legacy port IO ops can complete.
 */
static inline void 
___io_wait(void)
{
    /* Port 0x80 is used for 'checkpoints' during POST. */
    /* The Linux kernel seems to think it is free for use :-/ */
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

void kpanic(char* message);


#endif
