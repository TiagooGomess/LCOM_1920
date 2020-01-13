#include <lcom/lcf.h>

#include <lcom/lab3.h>


#include <stdbool.h>
#include <stdint.h>
#include "keyboard.h"
#include "timer.c"
#include "i8254.h"


extern int counter;
extern int sysinbcnt;
extern uint8_t scancode;
extern bool kbc_ih_valid;
extern bool make;
extern int size;


extern uint8_t bytes[];


int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {

    int ipc_status;
    int irq_set;
    uint8_t r;
    message msg;

    if (keyboard_subscribe_int(&r) != 0)   // subscreve as notificações
        return 1;

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
                            kbd_print_scancode(!make, size, bytes); // apenas para o lab3
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

    if (keyboard_unsubscribe_int() != 0)  // cancela subscrição das notificações
        return 1;

    // Imprimir o número de sys_inb
    if(kbd_print_no_sysinb(sysinbcnt) != 0) // apenas para o lab3
        return 1;

    return 0;
}


int(kbd_test_poll)() {

    uint8_t status;

    while(scancode != ESC_BREAK) {
        if (util_sys_inb(STATUS_REGISTER, &status) != 0)
            return 1;
    // Loop enquanto o i8042 output buffer está vazio
        if (status & OBF) {
            if (util_sys_inb(OUTPUT_BUFFER, &scancode) != 0)    // ler o output buffer(scancode)
                return 1;

            if (((status & PARITY) == 0) && ((status & TIMEOUT) == 0)) { //Bit da paridade e do timeout não podem ser 1, senão há erro
                make = scancode >> 7; // o bit mais significativo do scancode determina se é make ou breakcode
                uint8_t scanb = (uint8_t) scancode;
                kbd_print_scancode(!make, 1, &scanb); // apenas para o lab3
            }
        }
        tickdelay(micros_to_ticks(DELAY_US));
    }

    if (write_KBC_command_byte() != 0)  // escrever um comando KBC apropriado
        return 1;

    // Imprimir o número de sys_inb
    if(kbd_print_no_sysinb(sysinbcnt) != 0) // apenas para o lab3
        return 1;

    return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {

    int ipc_status;
    int irq_set_kbd;
    int irq_set_timer;
    uint8_t r;
    uint8_t num_seconds = 0;
    message msg;

    if(timer_subscribe_int(&r) != 0)  // subscrever as notificações do timer
        return 1;
    irq_set_timer = BIT(r);

    if (keyboard_subscribe_int(&r) != 0)   // subscrever as notificações do teclado
        return 1;
    irq_set_kbd = BIT(r);

    // Loop para tratar das interrupções, que termina quando o scancode for o breakcode da tecla ESC (0x81)
    // ou quando o número de segundos for maior ou igual ao parâmetro n
    while((scancode != ESC_BREAK) && (num_seconds < n))
    {
        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0)
        {
            printf("driver_receive failed with: %d", r);
            continue;
        }

        // Verificar se a interrupção é do teclado ou do timer
        if (is_ipc_notify(ipc_status))
        {
            switch (_ENDPOINT_P(msg.m_source))
            {
                case HARDWARE:

                    if (msg.m_notify.interrupts & irq_set_kbd)
                    {
                        kbc_ih();
                        if (!kbc_ih_valid)
                            break;
                        counter = 0;
                        num_seconds = 0;
                        if(kbd_print_scancode(!make, size, bytes) != 0) // apenas para o lab3
                            return 1;
                        size = 0;
                    }

                    if (msg.m_notify.interrupts & irq_set_timer)
                    {

                        timer_int_handler();

                        if(counter % 60 == 0) // verificar se passou um segundo
                        {
                            counter = 0;
                            num_seconds++;
                        }
                    }

                    break;

                default:
                    break;
            }
        }
        else
        {
        }
    }


    // Imprimir o número de sys_inb
    if(kbd_print_no_sysinb(sysinbcnt) != 0)  // apenas para o lab3
        return 1;

    if (keyboard_unsubscribe_int() != 0)  // retirar subscrição das notificaçõs do teclado
        return 1;

    if(timer_unsubscribe_int() != 0)  // retirar subscrição das notificaçõs do timer
        return 1;

    return 0;
}
