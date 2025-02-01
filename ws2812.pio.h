#pragma once

#include "hardware/pio.h"

extern const pio_program_t ws2812_program;

pio_sm_config ws2812_program_get_default_config(uint offset);
void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, uint freq, bool rgbw);
