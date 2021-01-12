#ifndef pic_H
#define pic_H

#include "kernel.h"

#define _PIC1		    0x20		/* IO base address for master PIC */
#define _PIC2		    0xA0		/* IO base address for slave PIC */
#define _PIC1_COMMAND	_PIC1
#define _PIC1_DATA	    (_PIC1+1)
#define _PIC2_COMMAND	_PIC2
#define _PIC2_DATA	    (_PIC2+1)

#define _PIC_EOI		0x20	    /* End-of-interrupt command code */

#define _ICW1_ICW4	    0x01		/* ICW4 (not) needed */
#define _ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define _ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define _ICW1_LEVEL	    0x08		/* Level triggered (edge) mode */
#define _ICW1_INIT	    0x10		/* Initialization - required! */

#define _ICW4_8086	    0x01		/* 8086/88 (MCS-80/85) mode */
#define _ICW4_AUTO	    0x02		/* Auto (normal) EOI */
#define _ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define _ICW4_BUF_MASTR	0x0C		/* Buffered mode/master */
#define _ICW4_SFNM	    0x10		/* Special fully nested (not) */

/*
 * Remaps pic IRQ vectors. Example: 
 *  master IRQ0 -> master_offset + 0
 *  master IRQ1 -> master_offset + 1 
 *  ...
 */
void pic_remap(int master_offset, int slave_offset);

/* 
 * Masks in interrupt line so that it won't raise irqs. 
 */
void pic_noirq(uint8_t irqline);

/* Clears an interrupt line so that it will raise irqs */
void pic_yesirq(uint8_t irqline);

/*
 * claers all interrupt lines on slave and master except for
 * lines 2 on slave and 9 on master (to keep cascading)
 */
void pic_noirqs(void);

/*
 * Remaps PIC IRLs to IRQs such that: 
 *
 *  (master)            (slave)
 *  IRL0 --> IRQ32      IRL8 --> IRQ40
 *  IRL1 --> IRQ33      IRL9 --> IRQ41
 *  IRL3 --> IRQ35      IRL10 -> IRQ42
 *  IRL4 --> IRQ36      IRL11 -> IRQ43
 *  IRL5 --> IRQ37      IRL12 -> IRQ44
 *  IRL6 --> IRQ38      IRL13 -> IRQ45
 *  IRL7 --> IRQ39      IRL14 -> IRQ46
 *                      IRL15 -> IRQ47
 *  
 *  and then masks all IRLs.
 */
status_t init_pic(void);

#endif
