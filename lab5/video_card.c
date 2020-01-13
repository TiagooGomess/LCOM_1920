#include <lcom/lcf.h>
#include "video_card.h"

void *video_mem;
uint32_t horizontalResolution;
uint32_t verticalResolution;
uint32_t bitsPerPixel;
uint32_t redMask;
uint32_t greenMask;
uint32_t blueMask;
uint32_t redFieldPosition;
uint32_t greenFieldPosition;
uint32_t blueFieldPosition;
uint32_t memoryModel;

void* initialAdress;

struct minix_mem_range mr;


void *(vg_init)(uint16_t mode)
{
    vbe_mode_info_t info;

    if (vbe_get_mode_info(mode, &info) != 0)
        return NULL;

    // Vai buscar as informações necessárias de info
    horizontalResolution = info.XResolution;
    verticalResolution = info.YResolution;
    bitsPerPixel = info.BitsPerPixel;
    redMask = info.RedMaskSize;
    greenMask = info.GreenMaskSize;
    blueMask = info.GreenMaskSize;
    redFieldPosition = info.RedFieldPosition;
    greenFieldPosition = info.GreenFieldPosition;
    blueFieldPosition = info.BlueFieldPosition;
    memoryModel = info.MemoryModel;

    memset(&mr, 0, sizeof(mr));  // Mete tudo a zero na struct

    unsigned int vram_base = info.PhysBasePtr;
    unsigned int vram_size = verticalResolution * horizontalResolution * (bitsPerPixel/8);
    mr.mr_base = (phys_bytes)vram_base;
    mr.mr_limit = mr.mr_base + vram_size;

    int r;
    if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
        panic("sys_privctl (ADD_MEM) failed: %d\n", r);

    video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vram_size); // Mapeamento de memória
    if (video_mem == MAP_FAILED)
        panic("couldn't map video memory");

    // Mudança do modo gráfico
    struct reg86 r1;
    memset(&r1, 0, sizeof(r1));  // Mete tudo a zero na struct
    r1.ax = (uint16_t)((0x4F << 8) | 0x02);
    r1.bx = 1 << 14 | mode;
    r1.intno = 0x10;
    if (sys_int86(&r1) != OK)
    {
        printf("set_vbe_mode: sys_int86() failed \n");
        return NULL;
    }
    return video_mem;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color)
{
    char* p = initialAdress;
    p += y * horizontalResolution * (bitsPerPixel / 8);
    p += x * (bitsPerPixel / 8);  // p vai para o pixel inicial

    for (int i = 0; i < len; i++) {
        for (unsigned int j = 0; j < (bitsPerPixel / 8); j++) {
            *p = (uint8_t)(color >> (j*8)); // Mete o pixel com a cor pretendida (junção de red, green e blue; cada um destes ocupam 1 byte)
            p++;
        }
    }

    return 0;
}


int (vg_draw_rectangle)(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint32_t color) {
    for(int i = 0; i < height; i++) {
        vg_draw_hline(x, y, width, color);  // Desenha linhas horizontais ao longo do retângulo, para este ficar preenchido
        y++;
    }
    return 0;
}
