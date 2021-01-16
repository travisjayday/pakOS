#include "virt-mm.h"

/* Points to where the next driver space allocation should  be made */
uint8_t* __driver_space_head = NULL;

/* Example / Explanation for myself -- Adress mapping
 *
 * 0xffc00000 maps to pde[1023], which holds a pointer to pde itself. 
 * i.e. 0xffc00000 is mapped to base of our pde, *0xffc00000 == pde[0]. 
 * To access at any index, we can set the index as the middle 10 bits of 0xffc00000. 
 * Setting the middle 10 bits to 768, we get the resulting address 0xfff00000. 
 * pde[768] is the 4mb pool in which our kenrel code gets loaded, so we should see
 * our hardware addresses when we loop over the table at pde[768]
 
    Example: 

        uint32_t* table_768 = 0xfff00000; 
        // kernel is loaded at 1mb secondary offset e.g. at index 256, so here we see all the 
        // hwardware addressses from weere the kerenl actuall is, e.g. 0x00100000
        table_768[0]        // is zero
        table_768[256];     // is 0x00100000
        table_768[257];     // is 0x00101000
        table_768[258];     // is 0x00102000
 */


// maps a hw frame to logical address
void 
mapfr(void* hwaddr, void* lgaddr) 
{
    // get 10 msb bits
    uint32_t pde_index = (uint32_t) lgaddr >> 22;  

    // get 10 msb bits after the first 10 msb bits (middle 10 bits) 
    uint32_t pte_index = (((uint32_t) lgaddr) >> 12) & 0x3ff;

    // create the right virtual address that accesses our pte table
    uint32_t* pte_table = (uint32_t*) (__logical_pde_base | (pde_index << 12));

    // Now we need to access pte_table. However, it might not be mapped to 
    // physical memory yet, so we need to check if it has been or not. 
    // If it doesn't exist, we need to allocate it and map it so that it does exist.
    if ((pde_table[pde_index] & 0x1) == 0) {
        void* new_hw_table = hwpalloc(1);
        pde_table[pde_index] = (uint32_t) new_hw_table | 0x3;
        __native_flush_tlb_single((uint32_t) pte_table);
    }

    // now pte_table can be accessed for sure.
    // map the requested hardware address to the destination virtual address
    pte_table[pte_index] = (uint32_t) hwaddr | 0x3;

    /* vga_puthex(pde_index);newl;
    vga_puthex(pte_index);newl;
    vga_puthex(pte_table[pte_index]);*/

    __native_flush_tlb_single((uint32_t) pte_table + pte_index * 4);
}

// unamps a hw frame from this logical address
void 
umapfr(void* lgaddr)
{
    // get 10 msb bits
    uint32_t pde_index = (uint32_t) lgaddr >> 22;  

    // get 10 msb bits after the first 10 msb bits (middle 10 bits) 
    uint32_t pte_index = (((uint32_t) lgaddr) >> 12) & 0x3ff;

    // create the right virtual address that accesses our pte table
    uint32_t* pte_table = (uint32_t*) (__logical_pde_base | (pde_index << 12));

    // Check if the page we want to unmap is actually mapped in the first place. 
    // Here we check if it's mapped in the PDE (if we can actually access the table)
    if ((pde_table[pde_index] & 0x1) == 0) {
        // is unmapped, so do nothing
        return;
    }

    // If we got here, we can access the table so,
    // set the pte table to zero at right offset
    pte_table[pte_index] = 0x0;

    __native_flush_tlb_single((uint32_t) pte_table + pte_index * 4);
}

status_t 
mmap(void* addr, size_t length) 
{
    // get number of pages to allocate
    uint32_t pages = (length + 4095) / 4096; 

    // allocate pages in RAM
    void* hwaddr = hwpalloc(pages);

    // map pages to requested virtual address 
    for (uint32_t i = 0; i < pages; i++) 
        mapfr((uint8_t*) hwaddr + 4096 * i, addr + 4096 * i);

    return STATUS_OK;
}

status_t 
munmap(void* addr, size_t length) 
{
    // get number of pages to allocate
    uint32_t pages = (length + 4095) / 4096; 

    // convert logical address to hardware address
    uint32_t hwaddr = _lpa(addr); 

    // unamp logical frames
    for (uint32_t i = 1; i < pages; i++) umapfr(addr + 4096 * i);

    // frees the underlying memory
    return hwpfree((void*) hwaddr, pages); 
}

void*
maphwdev(uint32_t hwaddr, size_t pages)
{
    if (__driver_space_head == NULL) 
        __driver_space_head = (uint8_t*) __driver_space_start;

    uint8_t* lspace = __driver_space_head;    

    __driver_space_head += pages;

    for (size_t p = 0; p < pages; p++) 
        mapfr((void*) (hwaddr + 4096 * p), (void*) (lspace + 4096 * p));

    return lspace;
}


/* We have to: 
 *  1. Define `__kheap_min_size' and use our phymm to allocate that space in physical RAM.
 *  2. Map that physical space to `__kheap_start' which lives somewhere above our kernel. 
 *  3. Initialize kmalloc. 
 */ 
status_t 
init_virtmm(void) 
{
    // map the heap
   /* _kguard(mmap((void*) __kheap_start, __kheap_min_size));
 
    // zero out the heap space. probably unecesary but ok.
    for (uint32_t i = 0; i < __kheap_min_size / 4; i++) 
        *(((uint32_t*) __kheap_start) + i) = 0;
        */

    //_kguard(munmap(__kheap_start, __kheap_min_size));
    //_kguard(init_kmalloc(__kheap_start, __kheap_min_size));
    _kguard(init_kmalloc(__kheap_start, 0));

    // Map frames test (maps first two heap frames, tests virtual write access)
#if 0
    uint32_t a0 = (uint8_t*) hwadr_pte_heap;
    uint32_t a1 = (uint8_t*) hwadr_pte_heap + 4096;
    uint32_t t0 = __kheap_start;
    uint32_t t1 = __kheap_start + 4096;
    mapfr(a0, t0);
    mapfr(a1, t1); newl;
    newl; vga_print("Map "); vga_puthex(a0); vga_print(" "); vga_puthex(t0);
    newl; vga_print("Map "); vga_puthex(a1); vga_print(" "); vga_puthex(t1);
    uint32_t f0 = __kheap_start;
    uint32_t f1 = __kheap_start + 4096;
    newl; vga_print("Testing access to "); vga_puthex(f0); 
    newl; vga_print("Testing access to "); vga_puthex(f1); 
    *(uint32_t*)(f0) = 0;
    *(uint32_t*)(f1) = 0;
    uint32_t val = *(uint32_t*)(f1);

    // Result: if you got here without crashing, you gucci
#endif
    
    // Unamp frame test 
#if 0 
    for (uint32_t i = 1; i < num_pages; i++) umapfr(__kheap_start + 4096 * i);
#endif 

    return STATUS_OK;
}
