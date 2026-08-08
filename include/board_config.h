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

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

//Defines are overrided by CMake options

//#define CONSOLE_DEBUG 1

//#define VARIANT_32_BUTTONS_ENABLED 1
//#define VARIANT_64_BUTTONS_ENABLED 1
//#define VARIANT_128_BUTTONS_ENABLED 1

//Disabled in current application
#define SUPPORT_3_AXIS 1
//#define SUPPORT_9_AXIS 1
//#define SUPPORT_16_AXIS 1


#if defined(SUPPORT_3_AXIS)
#define EnabledAxisSupportCount 3
#elif defined(SUPPORT_9_AXIS)
#define EnabledAxisSupportCount 9
#elif defined(SUPPORT_16_AXIS)
#define EnabledAxisSupportCount 16
#else
#define EnabledAxisSupportCount 0
#endif

#endif // BOARD_CONFIG_H