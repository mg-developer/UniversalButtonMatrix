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
#include "shiftregister.h"

#if HOST_BUILD == 0 
#include "pico/stdlib.h"
#include "tusb.h"
#include "usb_descriptors.h"

using time_type = absolute_time_t;

#else
#include "key_host.h"
using time_type = TimePoint;
#endif

#include <array>
#include <cstdint>
#include <cstdio>

constexpr uint32_t CLK_TIME_US = 20; // 0.0003 s

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

// Input pins for reading button rows (stores each column state for given row)
// Theoretically, we can handle up to:
// 3*ShiftRegister 74LS573 (3*8bit) = 24 colums
// 10*RP2040-GPIOs = 10 rows
// 24*10 = 240 buttons but in practice, we can handle up to 128 as USB HID device can handle.

constexpr std::array<uint8_t, 10> row_button_pins_in =
{
    4, 5, 6, 7, 8,
    9, 10, 11, 12, 13
};

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
    if(button_pins[DATA] == pin)
    {
        VirtualGPIO_data_set(value);
    }
    if(button_pins[LATCH] == pin)
    {
        VirtualGPIO_latch_set(value);
    }
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
    gpio_put(button_pins[LATCH], true);
    sleep_us(CLK_TIME_US * 2);
    gpio_put(button_pins[LATCH], false);
}

void latch_open()
{
    gpio_put(button_pins[LATCH], true);
    sleep_us(CLK_TIME_US * 2);
}

void latch_release()
{
    gpio_put(button_pins[LATCH], false);
    sleep_us(CLK_TIME_US * 2);    
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
    for (uint32_t mask = 1u << (bit_count - 1); mask > 0; mask >>= 1)
    {
        bool state = (value & mask) != 0;

    //for (int bit = bit_count - 1; bit >= 0; --bit)
    //{
    //    bool state = (value >> bit) & 1;
        gpio_put(button_pins[DATA], state);
        clk();
    }
}

// Verify the state of a specific column by pushing it to the shift register 
// and reading the row inputs
std::array<bool, BUTTON_INPUT_ROWS> key_column_check(uint8_t col, uint8_t max_col)
{
    uint32_t reg = 1 << col;
    register_push(reg, max_col);
    std::array<bool, BUTTON_INPUT_ROWS> state{};

    latch_open();

    for (uint8_t i = 0; i < BUTTON_INPUT_ROWS; ++i)
    {
        state[i] = gpio_get(row_button_pins_in[i]);
    }

    latch_release();
    clr_all();

    return state;
}


void button_board_init()
{
    // Outputs
    for (auto pin : button_pins)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, false);
    }

    // Row-Inputs
    for (auto pin : row_button_pins_in)
    {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_down(pin);
    }

    gpio_put(button_pins[LATCH], true);
    gpio_put(button_pins[DATA], false);

}

void button_board_handle(ButtonState& bt_set)
{
#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0     
    static int loop_count = 0;
    time_type start = get_absolute_time();
#endif

    constexpr uint8_t ColumnsMax = BUTTON_OUTPUT_COLUMNS;
    constexpr uint8_t RowsMax = BUTTON_INPUT_ROWS;

    for (uint8_t c = 0; c < ColumnsMax; ++c)
    {
        uint16_t base = c * RowsMax;
        auto single_row_state = key_column_check(c, ColumnsMax);

        for (uint8_t r = 0; r < RowsMax; ++r)
            bt_set[base + r] = single_row_state[r];
    }

#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0  
    if(loop_count++ % 100 == 0)
    {
        cdc_log_fmt("- Register shift time:%lld[us] \r\n", (unsigned long long)absolute_time_diff_us(start, get_absolute_time()));
    }
#endif

#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 1
    static time_type previous_time = {};

    if (absolute_time_diff_us(previous_time, get_absolute_time()) > 1500000)
    {
        previous_time = get_absolute_time();

        // Print column state for each row
        for (uint8_t c = 0; c < ColumnsMax; ++c)
            cdc_log_fmt("%u ", c);
        cdc_log("\r\n");

        for (uint8_t r = 0; r < RowsMax; ++r)
        {
            for (uint8_t c = 0; c < ColumnsMax; ++c)
            {
                cdc_log_fmt("%c ", bt_set[c * RowsMax + r] ? 'T' : 'F');
            }
            cdc_log("\r\n");
        }
        cdc_log("\r\n");
    }
#endif
}
