#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "i8254.h"
#include "keyboard.h"
#include "mouse.h"

extern uint32_t numberOfPackets;
extern bool mouse_ih_valid;
extern uint8_t pacote_array[3];
extern uint8_t mouse_scancode;
extern uint8_t status_mouse;
extern int counter;  // de timer.c
int num_seconds = 0;

struct packet pacote;


// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");
  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {

    int ipc_status;
    int irq_set;
    uint8_t r;
    message msg;

    if (mouse_subscribe_int(&r) != 0)   // subscreve as notificações
        return 1;

    irq_set = BIT(r);

    if (our_mouse_enable_data_reporting() != 0) // ativa o data reporting em stream mode
        return 1;

    while (cnt > numberOfPackets) {  // Ciclo que só termina quando já não houverem mais packets para ler

        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {  // Verifica o tipo de notificação
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: // Notificação de interrupt de hardware

                if (msg.m_notify.interrupts & irq_set) {  // se for uma interrupção / notificação do rato

                        mouse_ih();  // função que trata das interrupções do rato

                        if (!mouse_ih_valid) // se não for válido, break
                            break;

                        if (packet_parsing(&pacote) == 0)
                            mouse_print_packet(&pacote);

                    }

                    break;

                default:
                    break;
            }
        }
    }

    if (mouse_reset_state() != 0)  // desativa o data reporting
        return 1;

    if (mouse_unsubscribe_int() != 0)  // cancela subscrição das notificações
        return 1;

    return 0;
}


int (mouse_test_remote)(uint16_t period, uint8_t cnt) {

    if (send_command_to_mouse(SET_REMOTE_MODE) != 0)  // muda o rato para remote mode
        return 1;

    long int ticks = period * 1000;  // 1000 ticks -> 1 ms

    while (cnt > numberOfPackets) {  // Ciclo que só termina quando já não houverem mais packets para ler

        send_command_to_mouse(READ_DATA);

        for (int i = 1; i <= 3; i++) {  // 1 loop para cada byte (3)

            if (util_sys_inb(STATUS_REGISTER, &status_mouse) != 0)  // Lê status register
                return 1;

            if (((status_mouse & OBF) == 0) || ((status_mouse & AUX) == 0))  // Se o out buffer não estiver vazio
                                                                             // ou se o byte que está no output buffer
                                                                             // vem do teclado e não do rato
                break;

            if (util_sys_inb(OUTPUT_BUFFER, &mouse_scancode) != 0)  // Lê output buffer
                return 1;

            if (((status_mouse & PARITY) != 0) || ((status_mouse & TIMEOUT) != 0)) // Verifica se existem erros (de paridade ou de timeout)
                break;

            if (packet_parsing(&pacote) == 0)
                mouse_print_packet(&pacote);

        }

        tickdelay(micros_to_ticks(ticks)); // espera period milissegundos

    }

    if (send_command_to_mouse(SET_STREAM_MODE) != 0)  // muda o rato para stream mode
        return 1;

    if (remote_mouse_reset_state() != 0)  // desativa o data reporting
        return 1;

    uint32_t comando = minix_get_dflt_kbc_cmd_byte();  // A função minix_get_dflt_kbc_cmd_byte retorna o command byte padrão do KBC

    if (sys_outb(INPUT_BUFFER_COMMAND, INPUT_BUFFER_DATA) != 0) //  para "avisar" que vai ser escrito um comando no input buffer
        return 1;

    if (sys_outb(INPUT_BUFFER_DATA, comando) != 0)  // escreve o comando no input buffer (0x60)
        return 1;

    return 0;
}

int (mouse_test_async)(uint8_t  idle_time) {
    int ipc_status, irq_set_timer, irq_set;
    uint8_t r;
    message msg;

    if(timer_subscribe_int(&r)!= 0)  //subscreve as notificações do timer
        return 1;
    irq_set_timer= BIT(r);

    if (mouse_subscribe_int(&r) != 0)   // subscreve as notificações do rato
        return 1;
    irq_set= BIT(r);

    if(our_mouse_enable_data_reporting()!= 0)  // ativa o data reporting em stream mode
        return 1;

    while(idle_time > num_seconds) {  // Ciclo que só termina quando  o parâmetro idle_time
                                      // for maior do que o número de segundos que passaram

        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {  // Verifica o tipo de notificação
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE: {
                    if (msg.m_notify.interrupts & irq_set) {  // se for for uma interrupção do rato

                        mouse_ih(); // função que trata das interrupções do rato

                        if(!mouse_ih_valid)  // se não for válido, break
                            break;

                        if (packet_parsing(&pacote) == 0)
                            mouse_print_packet(&pacote);

                        counter = 0;
                    }

                    if (msg.m_notify.interrupts & irq_set_timer) {  // se for uma interrupção do timer

                        timer_int_handler();

                        if(counter % 60 == 0)  // verifica se passou um minuto (60 interrupções)
                        {
                            counter = 0;
                            num_seconds++;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        else {

        }
    }

    if(timer_unsubscribe_int() != 0)  // retira a subscrição das notificações do timer
        return 1;

    if (mouse_reset_state() != 0)  // desativa o data reporting
        return 1;

    if(mouse_unsubscribe_int() != 0)  // retira a subscrição das notificações do rato
        return 1;

    return 0;
}




int (mouse_test_gesture)(uint8_t comp_x, uint8_t var) {  // comp_x -> componente dos x; var -> tolerância

    int ipc_status, irq_set;
    uint8_t r;
    message msg;

    bool condition_one = false;  // condition_one -> garante que chegue ao vértice com sucesso; fica a false, caso depois
                                 // de chegar ao vértice com sucesso, o movimento seguinte não seja feito de maneira correta

    bool condition_two = false;  // condition_two -> nenhum botão premido quando chega ao vértice

    bool condition_three = false;  // condition_three -> garante que o movimento desde o vértice até ao final seja feito de
                                   // maneira correta

    bool condition_four = false;  // nenhum botão premido no fim do movimento

    int16_t delta_x = 0, delta_y = 0;  // deslocamentos nos eixos dos x (delta_x) e dos y (delta_y)

    if (mouse_subscribe_int(&r) != 0) // subscreve notificações do rato
        return 1;

    irq_set=BIT(r);

    if (our_mouse_enable_data_reporting() != 0)  // ativa o data reporting em stream mode
        return 1;

    while(!condition_four) { // Termina quando nenhum nenhum botão estiver pressionado se chegar ao fim do movimento com sucesso

        if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
            printf("driver_receive failed with: %d", r);
            continue;
        }
        if (is_ipc_notify(ipc_status)) {  // Verifica o tipo de notificação
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set) { // se for uma interrupção do rato

                        mouse_ih();  // função que trata das interrupções do rato

                        if (!mouse_ih_valid)  // se não for válido, break
                            break;

                        if (packet_parsing(&pacote) == 0) {

                            mouse_print_packet(&pacote);

                            if (pacote.lb == 1 && pacote.mb == 0 && pacote.rb == 0) {  // Apenas o botão esquerdo premido (lb)
                                delta_x += pacote.delta_y;  // faz o deslocamento
                                delta_y += pacote.delta_y;

                                if (pacote.delta_x < 0)  // se o deslocamento for no sentido negativo do eixo dos x
                                    if (abs(pacote.delta_x) >= var) {  // se ultrapassar a tolerância
                                        condition_one = false;
                                        delta_x = 0;  // reinicia-se o delta-x e o delta_y
                                        delta_y = 0;
                                        break;
                                    }

                                if (pacote.delta_y < 0)  // se o deslocamento for no sentido negativo do eixo dos y
                                    if (abs(pacote.delta_y) <= var) {  // se ultrapassar a tolerância
                                        condition_one = false;
                                        delta_x = 0; // reinicia-se o delta-x e o delta_y
                                        delta_y = 0;
                                        break;
                                    }

                                if (delta_x >= comp_x && (delta_y / delta_x) >= 1)  // se o deslocamento for maior ou igual a
                                                                                    // comp_x e o declive for maior ou igual a 1
                                    condition_one = true;  // condition_one -> chegou ao vértice com sucesso
                            }

                            if (condition_one && pacote.lb == 0 && pacote.mb == 0 && pacote.rb == 0) {   // Nenhum botão pressionado no vértice
                                condition_two = true;
                                delta_x = 0; // reinicia-se o delta-x e o delta_y
                                delta_y = 0;
                            }

                            if (condition_two && pacote.lb == 0 && pacote.mb == 0 && pacote.rb == 1) {  // Apenas o botão direito premido (rb)
                                                                                                        // depois de cumprida condition_two
                                delta_x += pacote.delta_x;  // faz o deslocamento
                                delta_y += pacote.delta_y;

                                if (pacote.delta_x < 0)  // se o deslocamento for no sentido negativo do eixo dos x
                                    if (abs(pacote.delta_x) >= var) {   // se ultrapassar a tolerância
                                        condition_one = false;
                                        delta_x = 0;   // reinicia-se o delta-x e o delta_y
                                        delta_y = 0;
                                        break;
                                    }

                                if (pacote.delta_y > 0)  // se o deslocamento for no sentido positivo do eixo dos y
                                    if (abs(pacote.delta_y) >= var) {   // se ultrapassar a tolerância
                                        condition_one = false;
                                        delta_x = 0;  // reinicia-se o delta-x e o delta_y
                                        delta_y = 0;
                                        break;
                                    }

                                if (delta_x >= comp_x && (delta_y / delta_x) <= -1)  // se o deslocamento for maior ou igual a
                                                                                     // comp_x e o declive for menor ou igual a 1
                                    condition_three = true;
                            }

                            if (condition_three && pacote.lb == 0 && pacote.mb == 0 && pacote.rb == 0)  // Nenhum botão é pressionado
                                                                                                        // no fim do movimento
                                condition_four = true;
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

    if (mouse_reset_state() != 0)  // desativa o data reporting
        return 1;

    if(mouse_unsubscribe_int() != 0)  // retira subscrição das notificações do rato
        return 1;

    return 0;
}

