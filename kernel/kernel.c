#include "kernel.h"
#include "phy-mm.h"
#include "virt-mm.h"
#include "vga.h"
#include "kstdlib.h"
#include "kstdio.h"
#include "gdt.h"
#include "idt.h"
#include "cpu_traps.h"
#include "pic.h"
#include "pci.h"
#include "serial.h"

__isr(keypress) {
    vga_clear();
    vga_print("Pressed a key!");
    KBREAK;
}

void kernel_init(void) {
    _kguard(init_serial());     // Inits debug logging over serial port.

    vga_clear(); vga_print("Booting OS2 written by tjz...\n\r");

    _kguard(init_phymm());      // Init physical page frame allocator.
    _kguard(init_virtmm());     // Init paging service and heap allocator. 
    _kguard(init_gdt());        // Init flat 4GB GDT.
    _kguard(init_idt());        // Init empty IDT.
    _kguard(init_cpu_traps());  // Register internal CPU fault handlers.
    _kguard(init_pic());        // Remap PIC and clear all IRQ lines. 
    _kguard(init_pci());        // Populate PIC devices. 

    
    /* Enable Keyboard 
    idt_mapint(1 + 0x20, &keypress);
    pic_yesirq(1);*/

    __asm__("sti");             // Enable Interrupts.
    
    load_pci_drivers();

    vga_print("This is string");
    vga_print((char*) itoa(0xb23, 16));
    vga_print("\n\r");

    printk("Hello world 0x%x are here.\nNow it is %d", 0xdeadbeef, 123);

    printk("Hellooowww %s jesus", "world");

    //__asm__("cli");
    KBREAK;
    return;
}

void
_kguard (status_t status) 
{
    if (DEBUG == 0) return;
    if (status != STATUS_OK) 
    {
        char* error = "Undefined Error";
        vga_clear();
        switch (status) 
        {
            case STATUS_BORKD_MEM:
                error = "Borked Memory Error";
                break;
            case STATUS_DOUBLE_FREE:
                error = "Double Free Error";
                break;
            case STATUS_INVALID_MEM:
                error = "Invalid Memory Access Violation";
                break;
            case STATUS_FAILED_INIT:
                error = "Kernel Initialization Routine Failed";
                break;
            case STATUS_OUT_OF_MEM:
                error = "Kernel Ran Out Of Memory";
                break;
            case STATUS_INVALID_FREE:
                error = "Invalid Freeing Of Memory";
        }     
        vga_print("KGuard Panic\n\n\r");
        vga_print(error);
        __asm__("jmp .");
    }
}

void 
kpanic(char* message) 
{
    vga_clear();
    vga_print("Encountered Bruh Moment: ");
    vga_print(message);
    __asm__("cli");
    __asm__("hlt");
}

