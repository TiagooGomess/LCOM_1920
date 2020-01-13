#include <lcom/lcf.h>
#include <lcom/lab2.h>

#include <stdbool.h>
#include <stdint.h>

extern int counter; // contador de interrupções

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab2/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab2/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();


  return 0;
}

int(timer_test_read_config)(uint8_t timer, enum timer_status_field field) {

    if (timer < 0 || timer > 2) {  // Testa-se se o valor de timer é válido
        printf("Timer value is not valid!\n");
        return 1;
    }

    uint8_t conf; // Estado do timer selecionado

    if (timer_get_conf(timer, &conf) != 0)
        return 1;

    if (timer_display_conf(timer, conf, field) != 0)
        return 1;

    return 0;
}

int(timer_test_time_base)(uint8_t timer, uint32_t freq) {

    if (timer < 0 || timer > 2) {  // Testa-se se o valor de timer é válido
        printf("Timer value is not valid!\n");
        return 1;
    }

    if (timer_set_frequency(timer, freq) != 0)
        return 1;

    return 0;
}

int(timer_test_int)(uint8_t time) {

    int ipc_status;
    int irq_set = 1;
    uint8_t r;
    uint32_t sec = 0;  // número de segundos
    message msg;

    if (timer_subscribe_int(&r) != 0)   // subscreve as notificações
        return 1;

    while (sec < time) {  // Get a request message
        if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: // hardware interrupt notification
                    if (msg.m_notify.interrupts & irq_set) {  // subscribed interrupt;
                                                              // irq_set é uma máscara de 32 bits, com o bit
                                                              // zero a 1, e os restantes a 0; se o bit zero
                                                              // de msg.m_notify.interrupts estiver a 1, a
                                                              // condição é verdadeira (1), e são executadas
                                                              // as instruções dentro do bloco if
                        timer_int_handler();  // incrementa counter
                        if (counter % 60 == 0) { // verifica se passou 1 segundo
                            counter = 0;     // reinicia o valor do contador
                            sec++;  // aumenta o contador de segundos
                            timer_print_elapsed_time();
                        }
                    }
                    break;
                default:
                    break;  // no other notifications expected: do nothing
            }
        }
        else {  // received a standard message, not a notification
            // no standard messages expected: do nothing
        }
    }

    if (timer_unsubscribe_int() != 0)  // cancela subscrição das notificações
        return 1;

    return 0;
}
