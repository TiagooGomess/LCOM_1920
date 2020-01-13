
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>

#define INDEXED_COLOR 0x04
#define DIRECT_COLOR 0x06

void *(vg_init)(uint16_t mode);

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color);

int (vg_draw_rectangle)(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint32_t color);
