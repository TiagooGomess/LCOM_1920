// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include "video_card.h"
#include "keyboard.h"


extern uint32_t horizontalResolution;
extern uint32_t verticalResolution;
extern uint32_t bitsPerPixel;
extern uint32_t redMask;
extern uint32_t greenMask;
extern uint32_t blueMask;
extern uint32_t redFieldPosition;
extern uint32_t greenFieldPosition;
extern uint32_t blueFieldPosition;
extern uint32_t memoryModel;
extern uint8_t scancode;
extern bool kbc_ih_valid;
extern int size;
extern void* initialAdress;



// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int(video_test_init)(uint16_t mode, uint8_t delay) {

    vg_init(mode); // Mapeamento da memória e mudança do modo gráfico

    sleep(delay); // espera delay segundos

    if (vg_exit() != 0)  // Volta para modo de texto
        return 1;

    return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

    initialAdress = vg_init(mode);  // Mapeia a memória e muda o modo gráfico

    if((x <= horizontalResolution) && (y <= verticalResolution)) { // Verifica se o x e o y estão dentro da resolução
        if (x+width > horizontalResolution) // Se o retângulo pretendido ultrapassar a resolução, desenhá-lo até ao limite
            width = horizontalResolution - x;
        if (y+height > verticalResolution)
            height = verticalResolution - y;
        vg_draw_rectangle(x, y, width, height, color); // Desenha o retângulo
    }

    int ipc_status;
    int irq_set;
    uint8_t r;
    message msg;

    if (keyboard_subscribe_int(&r) != 0) {  // subscreve as notificações
        vg_exit(); // Se falhar volta para modo de texto
        return 1;
    }

    irq_set = BIT(r);

    while (scancode != ESC_BREAK) {  // Ciclo que só termina quando scancode for um esc_break(0x81)
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {  //Verifica o tipo de notificação
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: // Notificação de interrupt de hardware
                    if (msg.m_notify.interrupts & irq_set) {

                        kbc_ih();

                        if (kbc_ih_valid) {
                            size = 0;

                        } else
                            break;
                    }

                    break;

                default:
                    break;
            }
        }
    }

    if(vg_exit() != 0) // Volta para modo de texto
        return 1;

    if (keyboard_unsubscribe_int() != 0)  // cancela subscrição das notificações
        return 1;

    return 0;
}


int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

    initialAdress = vg_init(mode);  // Mapeia a memória e muda o modo gráfico

    uint16_t altura, comprimento;
    uint32_t cor, red, green, blue;

    if (no_rectangles != 0) {
        altura = verticalResolution / no_rectangles;
        comprimento = horizontalResolution / no_rectangles;
    }

    for (int col = 0; col < no_rectangles; col++) {  // percorre as colunas
        for (int row = 0; row < no_rectangles; row++) {  // percorre as linhas
            if (memoryModel == INDEXED_COLOR) // se o modo de cor for indexed
                cor = (first + (col * no_rectangles + row) * step) % (1 << bitsPerPixel);
            else if (memoryModel == DIRECT_COLOR) { // se o modo de cor for direct

                // NÃO ESTÁ A PASSAR NOS TESTES!!!! PERGUNTAR AO PROFESSOR!!!

                red = (first + col * step) % (1 << redMask);
                green = (first + row * step) % (1 << greenMask);
                blue = (first + (col + row) * step) % (1 << blueMask);
                cor = red | green | blue;

            }
            else {
                vg_exit();  // Se o modo de cor não for indexed nem direct, volta para modo de texto
                return 1;
            }
            vg_draw_rectangle(row*comprimento, col*altura, comprimento, altura, cor); // Desenha retângulo
        }
    }


    int ipc_status;
    int irq_set;
    uint8_t r;
    message msg;

    if (keyboard_subscribe_int(&r) != 0) {  // subscreve as notificações
        vg_exit(); // Se falhar volta para modo de texto
        return 1;
    }

    irq_set = BIT(r);

    while (scancode != ESC_BREAK) {  // Ciclo que só termina quando scancode for um esc_break(0x81)
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {  //Verifica o tipo de notificação
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: // Notificação de interrupt de hardware
                    if (msg.m_notify.interrupts & irq_set) {

                        kbc_ih();

                        if (kbc_ih_valid) {
                            size = 0;

                        } else
                            break;
                    }

                    break;

                default:
                    break;
            }
        }
    }

    if(vg_exit() != 0) // Volta para modo de texto
        return 1;

    if (keyboard_unsubscribe_int() != 0)  // cancela subscrição das notificações
        return 1;

    return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {

    uint8_t* initial_address = vg_init(0x105);  // mapeamento da memória e mudança do modo gráfico para 0x105

    enum xpm_image_type type = XPM_INDEXED;
    xpm_image_t img;
    uint8_t *sprite = xpm_load(xpm, type, &img);

    uint32_t altura = img.height, comprimento = img.width;

    uint8_t* p;

    for (unsigned int i = 0; i < altura; i++) {
        if (y + i >= verticalResolution)  // Se a figura passar o limite da resolução vertical, break
            break;
        for (unsigned int j = 0; j < comprimento; j++) {
            if (x + j >= horizontalResolution)  // Se a figura passar o limite da resolução horizontal, break
                break;
            p = initial_address + (((y + i) * horizontalResolution) + x + j);  // Vai para o pixel pretendido
            *p = sprite[i * comprimento + j];  // Desenha a figura (alterando a memória)
        }
    }

    int ipc_status;
    int irq_set;
    uint8_t r;
    message msg;

    if (keyboard_subscribe_int(&r) != 0) {  // subscreve as notificações
        vg_exit(); // Se falhar volta para modo de texto
        return 1;
    }

    irq_set = BIT(r);

    while (scancode != ESC_BREAK) {  // Ciclo que só termina quando scancode for um esc_break(0x81)
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {  //Verifica o tipo de notificação
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: // Notificação de interrupt de hardware
                    if (msg.m_notify.interrupts & irq_set) {

                        kbc_ih();

                        if (kbc_ih_valid) {
                            size = 0;

                        } else
                            break;
                    }

                    break;

                default:
                    break;
            }
        }
    }

    if(vg_exit() != 0) // Volta para modo de texto
        return 1;

    if (keyboard_unsubscribe_int() != 0)  // cancela subscrição das notificações
        return 1;

    return 0;

}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  /* To be completed */
  printf("%s(%8p, %u, %u, %u, %u, %d, %u): under construction\n",
         __func__, xpm, xi, yi, xf, yf, speed, fr_rate);

  return 1;
}

int(video_test_controller)() {
  /* To be completed */
  printf("%s(): under construction\n", __func__);

  return 1;
}
