#ifndef SERIAL_H
#define SERIAL_H

#include "kernel.h"
#include "kstring.h"

#define _COM1PORT 0x3f8          // COM1

status_t init_serial();

void serial_putchar(char a);
void serial_print(char* str);

#endif
