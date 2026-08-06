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

#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include "board_config.h"
#include <array>

static_assert(VARIANT_32_BUTTONS_ENABLED + VARIANT_64_BUTTONS_ENABLED + VARIANT_128_BUTTONS_ENABLED == 1, "Exactly one button variant must be enabled");

#if (VARIANT_32_BUTTONS_ENABLED)
#define BUTTON_OUTPUT_ROWS 6
#define BUTTON_INPUT_COLUMNS 5
#endif

#if (VARIANT_64_BUTTONS_ENABLED)
#define BUTTON_OUTPUT_ROWS 7
#define BUTTON_INPUT_COLUMNS 10
#endif

#if (VARIANT_128_BUTTONS_ENABLED)
#define BUTTON_OUTPUT_ROWS 13
#define BUTTON_INPUT_COLUMNS 10
#endif

using ButtonState = std::array<bool, BUTTON_OUTPUT_ROWS * BUTTON_INPUT_COLUMNS>;

void button_board_init();
void button_board_handle(ButtonState& bt_set);

#endif // SHIFTREGISTER_H