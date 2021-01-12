#include "kstring.h"

int 
strlen(const char* str) 
{
    int i = 0; 
    while (str[i++] != '\0'); 
    return i; 
}

void* 
memcpy(void* dest, const void* src, size_t n) 
{
    uint8_t* d = (uint8_t*) dest;
    uint8_t* s = (uint8_t*) src;
    for (uint8_t i = 0; i < n; i++) d[i] = s[i];
    return dest; 
}
