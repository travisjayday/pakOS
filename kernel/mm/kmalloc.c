#include "kmalloc.h"

/* Priavte variable definitions */
struct _malloc_h*   _first_blk     = NULL;
uint32_t            _heap_start    = 0x0;
size_t              _heap_size     = 0;
size_t              _min_heap_size = 0;

/* Internal */ 

/* Finds a suitable block that holds 'size' data */
struct _malloc_h*   __find_free_block(struct _malloc_h **last, size_t size);

/* Asks the OS for new vm when no suitable blocks were found */
struct _malloc_h*   __new_blockspace(struct _malloc_h* last, size_t size);

/* Heap trace */
void                __dump_heap(uint32_t start_offset, uint32_t rows);

#ifdef LINUX_MALLOC
/* 
 * If we're compiling malloc for Linux use, define these stubs to hook into
 * libc malloc calls using LD_PRELOAD environment variable.
 */
void  free(void* ptr)                   { kfree(ptr);                   }
void* malloc(size_t size)               { return kmalloc(size);         }
void* realloc(void* ptr, size_t size)   { return krealloc(ptr, size);   }
void* calloc(size_t nmemb, size_t size) { return kcalloc(nmemb, size);  }
#endif

/*============================================================================*/ 

struct _malloc_h*
__find_free_block(struct _malloc_h **last, size_t size) 
{
  struct _malloc_h* current = _first_blk;

  // go through the linked list and find big enough free block
  while (current && !(current->free == _MBLKFREE && current->size >= size)) {
    *last = current;
    current = current->next_block;
  }

  return current;
}

struct _malloc_h* 
__new_blockspace(struct _malloc_h* last, size_t size) 
{
    struct _malloc_h *block;
    size_t nbytes = size + sizeof(struct _malloc_h);
    nbytes = (nbytes + 0x1000) & ~0xfff;

#ifndef LINUX_MALLOC
    void* new_space_start = (void*) (_heap_start + _heap_size); 
    mmap(new_space_start, nbytes); 
    block = new_space_start;
    _heap_size += nbytes;
#ifdef DEBUG_MALLOC
    vga_print("* kmalloc: Expanding heap by ");
    vga_puthex(nbytes);
    vga_print(" ["); vga_puthex(_heap_start); vga_print(" - ");
    vga_puthex(_heap_start + _heap_size); vga_print("]");
    serial_print("\n\r");
#endif

#else
    block = sbrk(0);
    void* new_space_start = sbrk(nbytes);
    assert((void*)block == new_space_start); // Not thread safe.
#endif

    if (new_space_start == (void*) -1 || 
        new_space_start == NULL) {
        return NULL; // sbrk / vm memory allocation failed.
    }

    if (last) { // NULL on first request.
        last->next_block = block;
        block->prev_block = last;
    }
    else {
        block->prev_block = NULL;
    }

    block->size         = nbytes - sizeof(struct _malloc_h);
    block->next_block   = NULL;
    block->free         = _MBLKFREE;
    block->magic        = 0xdeadbeef;

    return block;
}

void*
kmalloc(size_t length)
{
    // 16 byte align allocation
    length = (length + 0xf) & ~0xf;

    struct _malloc_h* block;
    if (_first_blk == NULL) {
        if (length > _min_heap_size) _min_heap_size = length;
        block = __new_blockspace(NULL, _min_heap_size);
        _first_blk = block;

        // this will split the large pre-allocated block 
        return kmalloc(length);
    }
    else {

        struct _malloc_h* last  = _first_blk;
        block = __find_free_block(&last, length);

        if (block == NULL) {
            // couldn't find big enough free block, request more memory
            block = __new_blockspace(last, length);
        }
        else {
            // if we can save 128 bytes, we split the block and add an extra 
            // free block in the middle
            if (block->size > 256 + length + sizeof(struct _malloc_h)) {
                struct _malloc_h* squeeze = (struct _malloc_h*) 
                    (((uint8_t*) (block + 1)) + length); 

                squeeze->size = block->size - sizeof(struct _malloc_h) - length;
                squeeze->free = _MBLKFREE;
                squeeze->magic = 0xb00bbabe;

                squeeze->next_block = block->next_block;
                squeeze->prev_block = block;

                if (block->next_block) 
                    block->next_block->prev_block = squeeze;

                block->next_block = squeeze;

                block->size = length;
            }
        }
    }
    block->free = _MBLKINUSE;

    return (void*) (block + 1);
}

void* 
kcalloc(size_t nmemb, size_t size) 
{
    size_t length = nmemb * size;
    void* data = (void*) kmalloc(length);
#ifdef LINUX_MALLOC
    memset(data, 0, length);
#else
    for (size_t i = 0; i < length; i++) ((uint8_t*) data)[i] = 0;
#endif
    return (void*) data;
}

void* 
krealloc(void* ptr, size_t size) 
{
    if (ptr == NULL) {
        return kmalloc(size);
    }
    struct _malloc_h* block = ((struct _malloc_h*) ptr) - 1;

    if (!_mblk_valid(block)) {
        vga_print("Invalid block!"); 
        return NULL;
    }

    if (block->size >= size) {
        return ptr;
    }
       
    uint8_t* old_data = (uint8_t*) ptr;
    uint8_t* new_data = (uint8_t*) kmalloc(size);
    if (new_data == NULL) return NULL;
#ifdef LINUX_MALLOC
    memcpy(new_data, old_data, block->size);
#else
    for (size_t i = 0; i < block->size; i++) {
        new_data[i] = old_data[i]; 
    }
#endif
    kfree(old_data); 
    return new_data;
}

status_t
kfree(void* addr) 
{
    if (addr == NULL) return STATUS_OK;

    struct _malloc_h* block = ((struct _malloc_h*) addr) - 1;

    if (!_mblk_valid(block)) {
        vga_print("Freeing invalid block:"); 
        serial_print("* malloc: Tried freeing invalid block at ");
        vga_puthex((uint32_t) addr);
#ifdef LINUX_MALLOC
        char c = write(0, "Invalid Free", 12);
        (void) c;
        fflush(0);
#endif
        return STATUS_INVALID_FREE; 
    }

#ifdef LINUX_MALLOC
    assert(block->free == _MBLKINUSE);
#endif

    struct _malloc_h* next = block->next_block;
    struct _malloc_h* prev = block->prev_block;

    if (next != NULL && next->free == _MBLKFREE) {
        // the block we're freeing and the next block will be free, so merge
        size_t total_size = block->size + next->size;
        block->next_block = next->next_block;
        block->size = total_size;
    }
    if (prev != NULL && prev->free == _MBLKFREE) {
        // the block we're freeing and the previous block is free, so merge
        uint32_t total_size = block->size + prev->size;
        prev->next_block = block->next_block;
        prev->size = total_size;
        block = prev;
    }
    
    if (block->next_block != NULL) block->next_block->prev_block = block;
    if (block->prev_block != NULL) block->prev_block->next_block = block;

    block->free = _MBLKFREE;

    return STATUS_OK;
}


status_t
init_kmalloc(uint32_t kheap_start, size_t kheap_size) 
{
    _heap_start = kheap_start;    
    _min_heap_size = kheap_size;
    _heap_size = 0;

    // Three way simple malloc and free test. For a full test, compiile 
    // this module for linux and use it to replace the default libc 
    // implementation.
#if 0
    void* a = kmalloc(20); void* b = kmalloc(11); void* c = kmalloc(11);
#define test 5
#if test == 0
    _kguard(kfree(a)); _kguard(kfree(b)); _kguard(kfree(c));
#elif test == 1
    _kguard(kfree(a)); _kguard(kfree(c)); _kguard(kfree(b));
#elif test == 2
    _kguard(kfree(b)); _kguard(kfree(a)); _kguard(kfree(c));
#elif test == 3
    _kguard(kfree(b)); _kguard(kfree(c)); _kguard(kfree(a));
#elif test == 4
    _kguard(kfree(c)); _kguard(kfree(a)); _kguard(kfree(b));
#elif test == 5 _kguard(kfree(c)); _kguard(kfree(b)); _kguard(kfree(a));
#endif
    // Result: All memory should be freed after each test
#endif 

    void* a = kmalloc(11000);
    kfree(a);
    //__dump_heap(0, 4);
    //__dump_heap(0x1f60, 4);

    return STATUS_OK;
}

void
__dump_heap(uint32_t start_offset, uint32_t rows) 
{
    newl;
    newl;
    vga_print("Heap trace:\n\r");
    uint32_t* start = (uint32_t*) ((uintptr_t) _heap_start + start_offset); 
    uint32_t i = 0; 
    for (uint32_t r = 0; r < rows; r++) {
        vga_puthex((uintptr_t) (start) + i); vga_print(": ");
        for (uint32_t c = 0; c < 4; c++) {
            uint32_t val = *(start + i); 
            vga_puthex(val); vga_print(" ");
            i++;
        } 
        vga_print("\n\r");
    }
}
