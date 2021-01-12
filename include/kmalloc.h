#ifndef KMALLOC
#define KMALLOC

#include <stdint.h>
#include <stddef.h>

#ifndef LINUX_MALLOC
    #include "kernel.h"
    #include "virt-mm.h"
    #include "vga.h"
#else
    #include <unistd.h>
    #include <stdio.h>
    #include <assert.h>
    #include <string.h>
    #include <stdlib.h>
    #define status_t uint8_t 
    #define STATUS_OK 0
    #define STATUS_INVALID_FREE 1
    void vga_print(const char* s) { (void) s; }
    void vga_puthex(uint32_t h)   { (void) h; }
#endif

#define newl vga_print("\r\n")

#define _MBLKFREE 1
#define _MBLKINUSE 0

/*
 * This is the structure placed before new allocations to efficiently
 * allocate memory on the heap. 
 */
struct _malloc_h {
    uint32_t free;
    uint32_t magic; 
    struct _malloc_h* next_block;
    struct _malloc_h* prev_block;
    size_t size; 
};

/*
 * Checks if a given _malloc_h is a valid _malloc_h (not corrputed)
 */
#define     _mblk_valid(blk)                \
            (blk->magic == 0xdeadbeef ||    \
             blk->magic == 0xb00bbabe)

#define     _mindword(num) ((num) + 0xf) & ~0xf

/* 
 * The following heap allocation functions adhere to the default 
 * malloc-family behavior as defined in the libc standard
 */
void*       kmalloc(size_t length);
void*       krealloc(void* ptr, size_t size);
void*       kcalloc(size_t nmemb, size_t size);
status_t    kfree(void* addr);

/* Inits kmalloc for use as kernel heap in our OS */
status_t    init_kmalloc(uint32_t kheap_start, size_t kheap_size);
#endif
