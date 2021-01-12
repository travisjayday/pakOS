#include "vga.h"

uint8_t vga_cursor_r = 0;
uint8_t vga_cursor_c = 0; 

void 
vga_set_char(uint8_t r, uint8_t c, char ch) 
{
    uint8_t color = VGA_COLOR_WHITE | VGA_COLOR_BLACK << 4; 
    uint16_t val = (uint16_t) ch | (uint16_t) color << 8; 
    VGA_BASE[r * VGA_WIDTH + c] = val; 
} 

void 
vga_clear(void) 
{
    for (uint8_t r = 0; r < VGA_HEIGHT; r++) {
        for (uint8_t c = 0; c < VGA_WIDTH; c++) {
            vga_set_char(r, c, ' ');
        } 
    }  
    vga_cursor_r = 0; 
    vga_cursor_c = 0;
}

void 
vga_putchar(char c)
{
    if (c == '\n') {
        vga_cursor_r++; 
    }
    else if (c == '\r') {
        vga_cursor_c = 0; 
    }
    else {
        vga_set_char(vga_cursor_r, vga_cursor_c, c);

        if (++vga_cursor_c == VGA_WIDTH) 
            vga_cursor_c = 0; 
    }
}

void 
vga_puthex(uint32_t i) 
{
    uint8_t* p = (uint8_t*) &i; 

    vga_print("0x");
    for (int j = 3; j >= 0; j--) {
        uint8_t byte = p[j];
        char nibbles[2]; 
        nibbles[1] = byte & 0x0f;   // lower nibble 
        nibbles[0] = byte >> 4;     // upper nibble
        for (int k = 0; k < 2; k++) {
            if (nibbles[k] >= 0 && nibbles[k] <= 9) 
                nibbles[k] += '0';
            else 
                nibbles[k] += 'A' - 10; 
            vga_putchar((char) nibbles[k]);
        }
    }
}

void 
vga_print(char* str) 
{
    while (*str!= 0) {
        vga_putchar(*str++);
    }
}
