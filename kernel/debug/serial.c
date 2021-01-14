#include "serial.h"

status_t 
init_serial() 
{
   ___outb(_COM1PORT + 1, 0x00);    // Disable all interrupts
   ___outb(_COM1PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   ___outb(_COM1PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   ___outb(_COM1PORT + 1, 0x00);    //                  (hi byte)
   ___outb(_COM1PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   ___outb(_COM1PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   ___outb(_COM1PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   ___outb(_COM1PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   ___outb(_COM1PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty (i.e: not same byte as sent)
   if (___inb(_COM1PORT + 0) != 0xAE) {
      return STATUS_FAILED_INIT;
   }

   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   ___outb(_COM1PORT + 4, 0x0F);
   return STATUS_OK;
}

int __is_transmit_empty() {
   return ___inb(_COM1PORT + 5) & 0x20;
}

void serial_putchar(char a) {
   while (__is_transmit_empty() == 0);
   ___outb(_COM1PORT, a);
}

void serial_print(char* str) {
    uint8_t len = strlen(str);
    for (uint8_t i = 0; i < len; i++) serial_putchar(str[i]);
}
