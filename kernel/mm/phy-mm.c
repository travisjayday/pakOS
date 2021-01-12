#include "phy-mm.h"

uint8_t* __phymm_end;
uint8_t* _hwa_mmstart;
uint8_t* _hwa_inu_bitmap;
uint32_t* _hwas_bp = (uint32_t*) __kernel_end;
uint32_t* _hwas_sp;
uint32_t _hwas_ssize;

// internal 
void _print_mmap_entry(struct bios_mmap_entry* ptr);


// returns a hw pointer to a n * 4kb aligned contiguous piece of RAM 
void* 
hwpalloc(uint16_t n) 
{
    if (_hwas_sp == _hwas_bp) {
        return NULL;
    }

    uint32_t addr;

    if (n == 1) {
        // get a page that is not in use from the top of stack
        do { 
            addr = *_hwas_sp;
            _hwas_sp--;
        }
        while (_hwa_inubm_get(addr) == _PGUSE);

        // mark this address as in use
        _hwa_inubm_set(addr, _PGUSE);
    }
    else {
        uint32_t cons = 0; 
        uint32_t i;

        // find n consecutive frames
        for (i = 0; i < _hwas_ssize; i++) {
            if (cons == n) break;
            if (_hwa_inubm_get((uint32_t*) _hwas_bp[i]) == _PGFREE) cons++;
            else cons = 0; 
        }    

        // get address in which consecutive frames start
        addr = _hwas_bp[i - cons];

        // mark address and the next n pages as in use
        for (i = 0; i < n; i++) _hwa_inubm_set(addr + i * 4096, _PGUSE);
    }
    return (uint32_t*) addr; 
}

// frees n hw frames for future use, starting at address hwaddr
status_t 
hwpfree(void* hwaddr, uint16_t n) 
{
    uint8_t* addr = (uint8_t*) hwaddr;
    if ((addr < _hwa_mmstart) || (addr >= _hwa_mmstart + _hwas_ssize * 4096)) {
        // hwaddr is not in main memory range.
        return STATUS_INVALID_MEM;
    }

    if (_hwa_inubm_get(hwaddr) == _PGFREE) {
        // double free
        return STATUS_DOUBLE_FREE;
    }

    if (n == 1) {
        if (_hwas_sp < _hwas_bp + _hwas_ssize) {
            // stack overrun;
            return STATUS_BORKD_MEM;
        }
        _hwa_inubm_set(addr, _PGFREE);
        _hwas_sp++;
        *_hwas_sp = (uint32_t) hwaddr;
    }
    else {
        for (uint32_t i = 0; i < n; i++) {
            _hwa_inubm_set(addr, _PGFREE);
            addr += 4096;
        }
    }

    return STATUS_OK;
}

status_t
init_phymm(void) 
{
    /*
    vga_print("bios mmap address: ");
    vga_puthex(bios_mmap_addr); 
    vga_print("\n\r");
    vga_print("bios mmap len: ");
    vga_puthex(bios_mmap_len); 
    vga_print("\n\r");

    vga_print("\n\r");
    */

    vga_print("Kernel ends at "); vga_puthex((uint32_t) __kernel_end); newl;
    struct bios_mmap_entry* mmap_ptr = (struct bios_mmap_entry*) bios_mmap_addr;
    uint8_t** byteptr = 0;
    do {
        _print_mmap_entry(mmap_ptr); 
        if (mmap_ptr->base_addr >= 0x100000 && mmap_ptr->type == 0x1) {
            // found a usable memory region 
            uint32_t kernel_size = (uint32_t) __kernel_end - 0xc0100000 + 0x1000;
            _hwa_mmstart = (uint8_t*) ((uint32_t) mmap_ptr->base_addr + kernel_size);
            _hwas_ssize = (mmap_ptr->length - kernel_size) / 4096;

            vga_print("Using memory region of size (pages): ");
            vga_puthex(mmap_ptr->length / 4096);
            vga_print(" at "); vga_puthex((uint32_t) _hwa_mmstart); newl;

            // lmiit max stack size for now
            if (_hwas_ssize > 128) {
                _hwas_ssize = 128;
            }

            // move all available hw pages onto the stack and set up the sp 
            uint32_t* stack_start = _hwas_bp;

            for (uint32_t i = 0; i < _hwas_ssize ; i++) 
                stack_start[i] = (uint32_t) ((uint8_t*) _hwa_mmstart + 4096 * i);
            _hwas_sp = _hwas_bp + _hwas_ssize - 1;

            // bitmap starts after stack ends. This is sketchy af.
            _hwa_inu_bitmap = (uint8_t*) (((uint32_t) _hwas_sp + 0xff) & ~0xff); 
            for (uint32_t i = 0; i < _hwa_inu_bitmap_size; i++) 
                _hwa_inu_bitmap[i] = _PGFREE;

            // calculate end of physical memory manager and page align it
            __phymm_end = (uint8_t*) ((uint32_t) _hwa_inu_bitmap + _hwa_inu_bitmap_size); 
            __phymm_end += 4096;
            __phymm_end = (uint8_t*) ((uint32_t) __phymm_end & ~0x3ff);

            // Single page free test
#if 0 
            uint32_t* p1 = hwpalloc(1);
            uint32_t* p2 = hwpalloc(1);
            uint32_t* p3 = hwpalloc(1);
            newl;
            vga_print("Got p1 from "); vga_puthex((uint32_t)p1); newl;
            vga_print("Got p2 from "); vga_puthex((uint32_t)p2); newl;
            vga_print("Got p3 from "); vga_puthex((uint32_t)p3); newl;
            vga_print("Freed p1 with status "); vga_puthex(hwpfree(p1, 1)); newl;
            vga_print("Freed p3 with status "); vga_puthex(hwpfree(p3, 1)); newl;
            p1 = hwpalloc(1);
            p3 = hwpalloc(1);
            vga_print("Got p1 from "); vga_puthex((uint32_t)p1); newl;
            vga_print("Got p3 from "); vga_puthex((uint32_t)p3); newl;
            // Result: p1 and p3 will switch addresses after freeing
#endif 

            // Multi page free test
#if 0
            uint8_t* p1 = hwpalloc(3);
            uint8_t* p2 = hwpalloc(1);
            uint8_t* p3 = hwpalloc(2); newl;
            vga_print("Got p1 from "); vga_puthex((uint32_t)p1); newl;
            vga_print("Got p2 from "); vga_puthex((uint32_t)p2); newl;
            vga_print("Got p3 from "); vga_puthex((uint32_t)p3); newl;
            vga_print("Freed p1 with status "); vga_puthex(hwpfree(p1, 3)); newl;
            uint32_t* p4 = hwpalloc(2); 
            vga_print("Got p4 from "); vga_puthex((uint32_t)p4); newl;
            newl;
            for (int i = 0; i < 7; i++) {
                uint32_t* hw = (uint32_t*)_hwas_bp[i];
                vga_puthex(hw); vga_print("   ");
                vga_puthex(_hwa_inubm_get(hw)); vga_print(" ");
                newl;
            }
            // Result should be 1 1 0 1 1  
#endif 

            // Single page alloc test
#if 0
            // the following two statements are equivalent 
            uint32_t* _adr = (uint32_t*)_hwas_bp[_hwas_ssize - 2];
            if (_adr != (uint32_t*)*(_hwas_sp - 1)) { mempanic(); }
            newl;
            vga_print("Marking "); vga_puthex(_adr); vga_print(" as used\n\r");
            vga_print("Marking "); vga_puthex(_adr + 4096); vga_print(" as used\n\r");
            _hwa_inubm_set(_adr, _PGUSE);
            _hwa_inubm_set(_adr + 4096, _PGUSE);
            newl; vga_puthex(_hwa_inu_bitmap_size); 
            newl; vga_puthex(_hwa_inu_bitmap[_hwa_inu_bitmap_size - 1]); // should be 1
            newl; vga_puthex(_hwa_inubm_get(_adr)); newl; 
            vga_print("Got page from "); vga_puthex(hwpalloc(1)); newl;
            vga_print("Got page from "); vga_puthex(hwpalloc(1)); newl;
            vga_print("Got page from "); vga_puthex(hwpalloc(1));
            // result should be such that we're not getting pages from marked indices
#endif 

            // Multipage alloc test 
#if 0
            _hwa_inubm_set((uint32_t*)_hwas_bp[1], 1);
            _hwa_inubm_set((uint32_t*)_hwas_bp[3], 1);
            newl;newl; vga_puthex(_hwa_inu_bitmap[0]);
            hwpalloc(3); newl;
            for (int i = 0; i < 7; i++) {
                uint32_t* hw = (uint32_t*)_hwas_bp[i];
                vga_puthex(hw); vga_print("   ");
                vga_puthex(_hwa_inubm_get(hw)); vga_print(" ");
                newl;
            }
            // -> result should be 0 1 0 1 1 1 1
#endif
            return STATUS_OK;
        }
        byteptr = (uint8_t**) &mmap_ptr;
        *byteptr += mmap_ptr->entry_size + 4; 
    } while (*byteptr <= (uint8_t*) bios_mmap_addr + bios_mmap_len);

    return STATUS_FAILED_INIT;
}

void 
_print_mmap_entry(struct bios_mmap_entry* ptr) 
{
    return;
    vga_print("entry_size: ");
    vga_puthex((uint32_t) ptr->entry_size);
    vga_print("\n\r");
    vga_print("base_addr: ");
    vga_puthex((uint32_t) ptr->base_addr);
    vga_print("\n\r");
    vga_print("length: ");
    vga_puthex((uint32_t) ptr->length);
    vga_print("\n\r");
    vga_print("type: ");
    vga_puthex((uint32_t) ptr->type);
    vga_print("\n\r");
}
