#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15


static const uint8_t VGA_WIDTH = 80;
static const uint8_t VGA_HEIGHT = 25;
static uint16_t* const VGA_BASE = (uint16_t*) 0xc00ff000;

extern uint8_t vga_cursor_r;
extern uint8_t vga_cursor_c; 

void vga_set_char(uint8_t r, uint8_t c, char ch);
void vga_clear(void);
void vga_putchar(char c);
void vga_puthex(uint32_t i);
void vga_print(char* str);
#endif
