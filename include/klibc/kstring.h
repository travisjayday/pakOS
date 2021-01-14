#ifndef KSTRING_H 
#define KSTRING_H 

#include <stddef.h>
#include <stdint.h>

#define status_t uint8_t
#include "kstdlib.h"
#include "kstdio.h"

/* As defined in standard C */
int strlen(const char* str);

/* 
 * As defined in standard C 
 * TODO: Please optimize me. 
 */
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* dest, uint8_t val, size_t n);

#endif
