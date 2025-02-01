#include "ws2812.pio.h"

// Definição do programa PIO para controlar os LEDs WS2812
const uint16_t ws2812_program_instructions[] = {
    // Instruções do programa PIO para controlar os LEDs WS2812
    0x6221, // out x, 1
    0x1123, // jmp !x, 3
    0x1400, // jmp 0
    0xa442, // nop
};

const pio_program_t ws2812_program = {
    .instructions = ws2812_program_instructions,
    .length = sizeof(ws2812_program_instructions) / sizeof(ws2812_program_instructions[0]),
    .origin = -1,
};

pio_sm_config ws2812_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset, offset + ws2812_program.length - 1);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}

void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, uint freq, bool rgbw) {
    pio_sm_config c = ws2812_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_out_shift(&c, false, true, 24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}
