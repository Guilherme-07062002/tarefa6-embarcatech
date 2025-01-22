#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

// Protótipos de função
void display_message_sinal_verde(uint8_t *ssd, struct render_area *frame_area);
void display_message_sinal_amarelo(uint8_t *ssd, struct render_area *frame_area);
void display_message_sinal_vermelho(uint8_t *ssd, struct render_area *frame_area);

// Definições de pinos
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

// Definição do botão A
#define BTN_A_PIN 5

// Variáveis globais
int A_state = 0;    // Armazena o estado do botão A
// Variáveis para o display
uint8_t *ssd;
struct render_area frame_area; 

// Funções
void SinalAberto(){
    // Exibir mensagem no display
    display_message_sinal_verde(ssd, &frame_area);

    // Acender LED verde e apagar os outros
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);   
}

void SinalAtencao(){
    // Exibir mensagem no display
    display_message_sinal_amarelo(ssd, &frame_area);

    // Acender LED amarelo e apagar os outros
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
}

void SinalFechado(){
    // Exibir mensagem no display
    display_message_sinal_vermelho(ssd, &frame_area);

    // Acender LED vermelho e apagar os outros
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}

int WaitWithRead(int timeMS){
    for(int i = 0; i < timeMS; i = i+100){
        A_state = !gpio_get(BTN_A_PIN);
        if(A_state == 1){
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}

// Definições de pinos I2C
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

void display_message_sinal_verde(uint8_t *ssd, struct render_area *frame_area) {
    // Limpar o display
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, frame_area);

    // Exibir mensagem
    char *text[] = {
        "SINAL ABERTO     ",
        "ATRAVESSAR COM",
        "CUIDADO"};

    int y = 0;
    for (uint i = 0; i < count_of(text); i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 8;
    }
    render_on_display(ssd, frame_area);
}

void display_message_sinal_amarelo(uint8_t *ssd, struct render_area *frame_area) {
    // Limpar o display
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, frame_area);

    // Exibir mensagem
    char *text[] = {
        "SINAL DE ",
        "ATENCAO     ",
        "PREPARE-SE        ",};

    int y = 0;
    for (uint i = 0; i < count_of(text); i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 8;
    }
    render_on_display(ssd, frame_area);
}

void display_message_sinal_vermelho(uint8_t *ssd, struct render_area *frame_area) {
    // Limpar o display
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, frame_area);

    // Exibir mensagem
    char *text[] = {
        "SINAL FECHADO     ",
        "AGUARDE       "};

    int y = 0;
    for (uint i = 0; i < count_of(text); i++) {
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 8;
    }
    render_on_display(ssd, frame_area);
}


int main() {
    stdio_init_all();   // Inicializa os tipos stdio padrão presentes ligados ao binário

    // Inicialização do i2c
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();

    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    frame_area = (struct render_area) {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Zera o display inteiro
    ssd = malloc(ssd1306_buffer_length);
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Iniciando leds
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Iniciando botäo
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    while(true){
        SinalFechado();
        A_state = WaitWithRead(8000);   // Espera leitura do botão A por 8s

        if(A_state){               // Se foi apertado o botão, mudar ciclo
            // Liga sinal amarelo por 5s
            SinalAtencao();
            sleep_ms(5000);

            // Liga sinal verde por 10s
            SinalAberto();
            sleep_ms(10000);

        }else{                          // Se ninguém apertou o botão, segue o ciclo normal
            // Liga sinal amarelo por 2s
            SinalAtencao();
            sleep_ms(2000);

            // Liga sinal verde por 8s
            SinalAberto();
            sleep_ms(8000);
        }
                
    }

    return 0;
}