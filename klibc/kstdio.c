#include "kstdio.h"

void printk(const char *fmt, ...) {
    va_list argptr; 
    va_start(argptr, fmt); 
    uint16_t len = strlen(fmt);
    char* buffer = (char*) kmalloc(200);
    uint16_t bufidx = 0;
    for (uint16_t i = 0; i < len; i++) {
        if (fmt[i] == '%' && i < len - 1) {
            // handle special format 
            i++;

            // print int 
            if (fmt[i] == 'd' || fmt[i] == 'x') {
                uint8_t base = fmt[i] == 'd'? 10 : 16;
                char* intstr = itoa(va_arg(argptr, uint32_t), base);
                uint8_t intstrlen = strlen(intstr) - 1;
                memcpy(buffer + bufidx, intstr, intstrlen);
                bufidx += intstrlen;
                kfree(intstr);
            }

            // print long long int
            if (fmt[i] == 'l' && (fmt[i+1] == 'x' || fmt[i+1] == 'd')) {
                i++;
                uint8_t base = fmt[i] == 'd'? 10 : 16;
                char* intstr = itoa(va_arg(argptr, uint64_t), base);
                uint8_t intstrlen = strlen(intstr) - 1;
                memcpy(buffer + bufidx, intstr, intstrlen);
                bufidx += intstrlen;
                kfree(intstr);
            }
        }
        else if (fmt[i] == '\n') {
            buffer[bufidx++] = '\n';
            buffer[bufidx++] = '\r';
        }
        else {
            buffer[bufidx++] = fmt[i];
        }
    }
    buffer[bufidx] = '\0';
    vga_print(buffer);
    kfree(buffer);
    va_end(argptr); 
}
