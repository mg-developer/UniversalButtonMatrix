/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Michał Głuszek (sysprog.pl)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "analogaxis.h"

#if HOST_BUILD == 0
#include <pico/stdlib.h>
#include <hardware/adc.h>
#endif

static constexpr uint8_t ANALOG_X_PIN = 29;
static constexpr uint8_t ANALOG_Y_PIN = 28;
static constexpr uint8_t ANALOG_X_CHANNEL = 3;
static constexpr uint8_t ANALOG_Y_CHANNEL = 2;

static int adc_raw_to_axis(uint16_t raw)
{
    return static_cast<int>((raw + 8u) >> 4u);
}

void analog_board_init()
{
#if HOST_BUILD == 0
    adc_init();
    adc_gpio_init(ANALOG_X_PIN);
    adc_gpio_init(ANALOG_Y_PIN);
#endif
}

void analog_board_handle(AnalogState& ax_set)
{
#if HOST_BUILD == 0
    adc_select_input(ANALOG_X_CHANNEL);
    const uint16_t raw_x = adc_read();

    adc_select_input(ANALOG_Y_CHANNEL);
    const uint16_t raw_y = adc_read();

    if (ax_set.size() > 0u)
        ax_set[0] = adc_raw_to_axis(raw_x);
    if (ax_set.size() > 1u)
        ax_set[1] = adc_raw_to_axis(raw_y);
    if (ax_set.size() > 2u)
        ax_set[2] = 128; //Unused axis, set to center position
#else
    for (auto &value : ax_set)
        value = 128; //Unused axis, set to center position
#endif
}
