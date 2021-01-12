#include "kstdlib.h"

char* itoa(uint64_t value, int base) {
    uint8_t buflen = 32;
    char* s = kcalloc(buflen, 1);
    uint8_t i = 0;
    s[buflen--] = '\0';

    uint32_t lower = (uint32_t) value;
    uint32_t upper = (uint32_t) (value >> 32);
    for (int _ = 0; _ < 2; _++) {
        uint32_t val = _ == 0? lower : upper; 
        while (val > 0) {
            uint8_t digit = val % base;
            if (digit < 10) digit += '0';
            else            digit += 'A' - 10;
            s[buflen - i++] = digit;
            val /= base; 
        }
    }
    memcpy(s, s + buflen - i + 1, i);
    s[i] = '\0';
    return s;
}
