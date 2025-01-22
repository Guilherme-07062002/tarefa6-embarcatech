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

// Definição de botão
#define BTN_A_PIN 5

// Variáveis globais
int A_state = 0;    //Botao A está pressionado?
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

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

void display_message_sinal_verde(uint8_t *ssd, struct render_area *frame_area) {
    // Limpar o display
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, frame_area);

    // Exibir mensagem no display
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

    // Exibir mensagem no display
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

    // Exibir mensagem no display
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

    // zera o display inteiro
    ssd = malloc(ssd1306_buffer_length);
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // INICIANDO LEDS
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // INICIANDO BOTÄO
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    while(true){
        SinalFechado();
        A_state = WaitWithRead(8000);   //espera com leitura do botäo

        if(A_state){               //ALGUEM APERTOU O BOTAO - SAI DO SEMAFORO NORMAL
            //SINAL AMARELO PARA OS CARROS POR 5s
            SinalAtencao();
            sleep_ms(5000);

            //SINAL VERMELHO PARA OS CARROS POR 10s
            SinalAberto();
            sleep_ms(10000);

        }else{                          //NINGUEM APERTOU O BOTAO - CONTINUA NO SEMAFORO NORMAL
            SinalAtencao();
            sleep_ms(2000);

            //SINAL VERMELHO PARA OS CARROS POR 15s
            SinalAberto();
            sleep_ms(8000);
        }
                
    }

    return 0;
}