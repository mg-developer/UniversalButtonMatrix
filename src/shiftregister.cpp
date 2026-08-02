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

#if HOST_BUILD == 0 
#include "pico/stdlib.h"
#else
#include "key_host.h"
#endif

#include "shiftregister.h"
#include <array>
#include <cstdint>

constexpr uint32_t CLK_TIME_US = 300; // 0.0003 s

#if HOST_BUILD == 1 
enum
{
    GPIO_IN = 0,
    GPIO_OUT = 1,
};
void sleep_us(uint32_t us)
{
    // Simulate sleep in host build (no-op)
}
void gpio_put(uint8_t pin, bool value)
{
    // Simulate GPIO output in host build (no-op)
}
uint16_t gpio_get(uint8_t pin)
{
    return VirtualGPIO_get(pin);
}
void gpio_init(uint8_t pin)
{
    // Simulate GPIO initialization in host build (no-op)
}
void gpio_set_dir(uint8_t pin, bool dir)
{
    // Simulate GPIO direction setting in host build (no-op)
}
void gpio_pull_down(uint8_t pin)
{
    // Simulate GPIO pull-down in host build (no-op)
}
#endif

enum
{
    LATCH = 0,
    CLR,
    DATA,
    CLK,
};

constexpr std::array<uint8_t, 4> button_pins =
{
    27, // LATCH
    26, // CLR
    15, // DATA
    14, // CLK
};

// Input pins for reading button (columns or rows)states
// Theoretically, we can handle up to:
// 3*ShiftRegister 74LS573 (3*8) = 24 colums
// 10*RP2040 GPIOs = 10 rows
// 24*10 = 240 buttons but in practice, we can handle up to 128 as USB HID device can handle.

constexpr std::array<uint8_t, 10> button_pins_in =
{
    4, 5, 6, 7, 8,
    9, 10, 11, 12, 13
};

void delay_clk()
{
    sleep_us(CLK_TIME_US);
}

void clk()
{
    delay_clk();
    gpio_put(button_pins[CLK], true);
    delay_clk();
    gpio_put(button_pins[CLK], false);
}

void latch()
{
    sleep_us(CLK_TIME_US * 3);
    gpio_put(button_pins[LATCH], true);
    sleep_us(CLK_TIME_US * 3);
    gpio_put(button_pins[LATCH], false);
}

void latch_open()
{
    sleep_us(CLK_TIME_US * 3);
    gpio_put(button_pins[LATCH], true);
    sleep_us(CLK_TIME_US * 3);
}

void latch_release()
{
    sleep_us(CLK_TIME_US * 3);
    gpio_put(button_pins[LATCH], false);
}

void clr()
{
    delay_clk();
    gpio_put(button_pins[CLR], false);
    delay_clk();
    gpio_put(button_pins[CLR], true);
}

void clr_all()
{
    delay_clk();
    gpio_put(button_pins[CLR], false);
    delay_clk();
    gpio_put(button_pins[CLR], true);
    delay_clk();
}

void register_push(uint32_t value, uint8_t bit_count)
{
    for (int bit = bit_count - 1; bit >= 0; --bit)
    {
        bool state = (value >> bit) & 1;
        gpio_put(button_pins[DATA], state);
        clk();
    }
}

void gpio_init_all_buttons()
{
    // Outputs
    for (auto pin : button_pins)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, false);
    }

    // Inputs
    for (auto pin : button_pins_in)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }

    gpio_put(button_pins[LATCH], true);
    gpio_put(button_pins[DATA], false);
}

std::array<bool, BUTTON_OUTPUT_ROWS> key_column_check(uint8_t col)
{
    uint32_t reg = 1 << col;
    register_push(reg, 32);
    std::array<bool, BUTTON_OUTPUT_ROWS> state{};

    latch_open();
    sleep_us(CLK_TIME_US * 8);

    for (uint8_t i = 0; i < BUTTON_OUTPUT_ROWS; ++i)
    {
        state[i] = gpio_get(button_pins_in[i]);
    }

    sleep_us(CLK_TIME_US * 2);
    latch_release();
    clr_all();

    return state;
}

void button_board_handle(ButtonState& bt_set)
{
    for (uint8_t c = 0; c < BUTTON_INPUT_COLUMNS; ++c)
    {
        uint16_t base = c * BUTTON_OUTPUT_ROWS;
        auto state = key_column_check(c);

        for (uint8_t r = 0; r < BUTTON_OUTPUT_ROWS; ++r)
            bt_set[base + r] = state[r];
    }

#if CONSOLE_DEBUG == 1
    for (uint8_t c = 0; c < BUTTON_INPUT_COLUMNS; ++c)
        printf("%u ", c);
    printf("\n");

    for (uint8_t r = 0; r < BUTTON_OUTPUT_ROWS; ++r)
    {
        for (uint8_t c = 0; c < BUTTON_INPUT_COLUMNS; ++c)
        {
            printf("%c ", bt_set[c * BUTTON_OUTPUT_ROWS + r] ? 'T' : 'F');
        }
        printf("\n");
    }
#endif
}
