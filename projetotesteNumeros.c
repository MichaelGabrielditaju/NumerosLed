#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"  // Inclui o programa WS2812
#include "numeros.h"  // Inclui a matriz de números

// Definições de pinos
#define LED_VERMELHO 13

#define BOTAO_A 5
#define BOTAO_B 6
#define DEBOUNCE_MS 200
#define WS2812_PIN 7

// Variável global para armazenar o número exibido
int numero_exibido = 0;
uint32_t tempo_ultimo_botao_a = 0;
uint32_t tempo_ultimo_botao_b = 0;    

// Protótipos das funções
void configurar_pinos();
void piscar_led_vermelho(PIO pio, uint sm);
void incrementar_numero(PIO pio, uint sm);
void decrementar_numero(PIO pio, uint sm);
void tratar_botao_a(PIO pio, uint sm);
void tratar_botao_b(PIO pio, uint sm);
void exibir_numero(int numero, PIO pio, uint sm);
void desenha_fig(const uint32_t *_matriz, uint8_t _intensidade, PIO pio, uint sm);
void ligar_matriz_branca(PIO pio, uint sm);
void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, uint freq, bool rgbw);

// Funções de interrupção
void gpio_callback(uint gpio, uint32_t events) {
    PIO pio = pio0; // ou pio1, dependendo do seu caso
    int sm = 0; // ou outro valor apropriado

    if (gpio == BOTAO_A) {
        tratar_botao_a(pio, sm);
    } else if (gpio == BOTAO_B) {
        tratar_botao_b(pio, sm);
    }
}

int main() {
    stdio_init_all();
    configurar_pinos();
    // Inicialize o PIO e o State Machine para os LEDs WS2812
    PIO pio = pio0;
    uint sm = 0;

    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
    while (true) {
        piscar_led_vermelho(pio, sm);
                
        sleep_ms(100);
    }
    return 0;
}

void configurar_pinos() {
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

void piscar_led_vermelho(PIO pio, uint sm) {
    static bool estado_led = false;
    static uint32_t ultimo_tempo = 0;
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    if (tempo_atual - ultimo_tempo >= 100) {  // Frequência de 5 Hz (100 ms)
        estado_led = !estado_led;
        gpio_put(LED_VERMELHO, estado_led);
        ultimo_tempo = tempo_atual;
    }
}

void ligar_matriz_branca(PIO pio, uint sm) {
    uint32_t branco = 0xFFFFFF; // Cor branca (RGB: 255, 255, 255)
    for (int i = 0; i < 25; i++) {
        pio_sm_put_blocking(pio, sm, branco << 8u);
    }
}

void incrementar_numero(PIO pio, uint sm) {
    if (numero_exibido < 9) {
        numero_exibido++;
        exibir_numero(numero_exibido, pio, sm);
    }
}

void decrementar_numero(PIO pio, uint sm) {
    if (numero_exibido > 0) {
        numero_exibido--;
        exibir_numero(numero_exibido, pio, sm);
    }
}

void tratar_botao_a(PIO pio, uint sm) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    if (tempo_atual - tempo_ultimo_botao_a >= DEBOUNCE_MS) {
        incrementar_numero(pio, sm);
        tempo_ultimo_botao_a = tempo_atual;
    }
}

void tratar_botao_b(PIO pio, uint sm) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    if (tempo_atual - tempo_ultimo_botao_b >= DEBOUNCE_MS) {
        decrementar_numero(pio, sm);
        tempo_ultimo_botao_b = tempo_atual;
    }
}

void exibir_numero(int numero, PIO pio, uint sm) {
    desenha_fig(numero_matrizes[numero], 100, pio, sm); // Passa a matriz correta
}
void desenha_fig(const uint32_t *_matriz, uint8_t _intensidade, PIO pio, uint sm) {
    uint32_t pixel = 0; uint8_t r, g, b;

    for (int i = 24; i > 19; i--) { // Linha 1
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 15; i < 20; i++) { // Linha 2
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (b << 16) | (r << 8) | g;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 14; i > 9; i--) { // Linha 3
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

    for (int i = 5; i < 10; i++) { // Linha 4
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }

        for (int i = 4; i > -1; i--) { // Linha 5
        pixel = _matriz[i];
        b = ((pixel >> 16) & 0xFF) * (_intensidade / 100.00); // Isola os 8 bits mais significativos (azul)
        g = ((pixel >> 8) & 0xFF) * (_intensidade / 100.00);  // Isola os 8 bits intermediários (verde)
        r = (pixel & 0xFF) * (_intensidade / 100.00);         // Isola os 8 bits menos significativos (vermelho)
        pixel = 0;
        pixel = (g << 16) | (r << 8) | b;
        pio_sm_put_blocking(pio, sm, pixel << 8u);
    }
}
