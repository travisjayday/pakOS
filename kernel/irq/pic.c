#include "pic.h"

void 
pic_remap(int master_offset, int slave_offset)
{
	uint8_t a1 = ___inb(_PIC1_DATA);                // save masks
	uint8_t a2 = ___inb(_PIC2_DATA);

	___outb(_PIC1_COMMAND, _ICW1_INIT | _ICW1_ICW4);  // starts the initialization sequence (in cascade mode
	___io_wait();
	___outb(_PIC2_COMMAND, _ICW1_INIT | _ICW1_ICW4);
	___io_wait();
	___outb(_PIC1_DATA, master_offset);            // ICW2: Master PIC vector offset
	___io_wait();
	___outb(_PIC2_DATA, slave_offset);             // ICW2: Slave PIC vector offset
	___io_wait();
	___outb(_PIC1_DATA, 4);                        // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	___io_wait();
	___outb(_PIC2_DATA, 2);                        // ICW3: tell Slave PIC its cascade identity (0000 0010) (IRQ line 9)
	___io_wait();

	___outb(_PIC1_DATA, _ICW4_8086);
	___io_wait();
	___outb(_PIC2_DATA, _ICW4_8086);
	___io_wait();

	___outb(_PIC1_DATA, a1);   // restore saved masks.
	___outb(_PIC2_DATA, a2);
}

void
pic_noirqs(void)
{
    ___outb(_PIC1_DATA, ~4);
    ___outb(_PIC2_DATA, ~2);
}

void 
pic_noirq(uint8_t irqline) 
{
    uint16_t port;
    uint8_t value;

    if(irqline < 8) {
        // line is on master
        port = _PIC1_DATA;
    } else {
        // line is on slave
        port = _PIC2_DATA;
        irqline -= 8;
    }
    value = ___inb(port) | (1 << irqline);
    ___outb(port, value);
}

void 
pic_yesirq(uint8_t irqline) 
{
    uint16_t port;
    uint8_t value;

    if (irqline < 8) {
        // line is on master
        port = _PIC1_DATA;
    } else {
        // line is on slave
        port = _PIC2_DATA;
        irqline -= 8;
    }
    value = ___inb(port) & ~(1 << irqline);
    ___outb(port, value);
}

status_t
init_pic(void)
{
/*    __asm__("mov $0xff, %al\n\t"
            "out %al, $0xa1\n\t"
            "out %al, $0x21\n\t");*/
    pic_remap(0x20, 0x28);
    pic_noirqs();

    return STATUS_OK;
}
