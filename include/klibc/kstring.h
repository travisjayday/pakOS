#ifndef KSTRING_H 
#define KSTRING_H 

#include <stddef.h>
#include <stdint.h>

/* As defined in standard C */
int strlen(const char* str);

/* 
 * As defined in standard C 
 * TODO: Please optimize me. 
 */
void* memcpy(void* dest, const void* src, size_t n);

#endif
