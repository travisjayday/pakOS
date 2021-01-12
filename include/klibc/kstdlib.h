#ifndef KSTDLIB_C
#define KSTDLIB_C

#include "kmalloc.h"
#include "kstring.h"

/*
 * Converts an integer to a C string.
 *
 * As defined in standard C. Remember it is the caller's responsibility
 * to kfree() the resulting const char* (frees 16 bytes).
 */ 
char* itoa(uint64_t value, int base);

#endif 
