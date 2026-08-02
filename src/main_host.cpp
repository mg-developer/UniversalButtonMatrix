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

#include "board_config.h"
#include "shiftregister.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <map>

#ifdef _WIN32
#include <conio.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

static_assert(VARIANT_32_BUTTONS_ENABLED + VARIANT_64_BUTTONS_ENABLED + VARIANT_128_BUTTONS_ENABLED == 1, "Exactly one button variant must be enabled");

#ifdef _WIN32
static int get_key_nonblocking()
{
    return _kbhit() ? _getch() : -1;
}
#else
static int get_key_nonblocking()
{
    static bool initialized = false;
    static struct termios oldt;

    if (!initialized)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        struct termios newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 0;
        newt.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        atexit([]() {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        });
        initialized = true;
    }

    char ch;
    int n = read(STDIN_FILENO, &ch, 1);
    return (n == 1) ? static_cast<unsigned char>(ch) : -1;
}
#endif

class VirtualGPIO
{
public:
    VirtualGPIO()
        : active_key(-1), key_register(0), stop_flag(false), input_thread(&VirtualGPIO::key_loop, this)
    {
        std::fill(std::begin(gpio_state), std::end(gpio_state), false);
    }

    ~VirtualGPIO()
    {
        stop_flag.store(true);
        if (input_thread.joinable())
        {
            input_thread.join();
        }
    }

    int get_key() const
    {
        return key_register.load();
    }

    bool has_key() const
    {
        return key_register.load() != 0;
    }

    void reset_key()
    {
        if (active_key != -1)
        {
            release_key(active_key);
        }
        key_register.store(0);
    }

    bool is_stopped() const
    {
        return stop_flag.load();
    }

    bool get_state(int key) const
    {
        if (key < 0 || key > 127)
            return false;
        return gpio_state[key];
    }

private:
    std::array<bool, 128> gpio_state;
    int active_key;
    std::atomic<int> key_register;
    std::atomic<bool> stop_flag;
    std::thread input_thread;

    void press_key(int key)
    {
        if (active_key != -1 && active_key != key)
        {
            release_key(active_key);
        }
        active_key = key;
        gpio_state[key] = true;
    }

    void release_key(int key)
    {
        if (key >= 0 && key < static_cast<int>(gpio_state.size()))
        {
            gpio_state[key] = false;
        }
        if (active_key == key)
        {
            active_key = -1;
        }
    }

    void key_loop()
    {
        std::cout << "Press keys 1-9 to store a key in the internal register, or q to quit.\n";

        while (!stop_flag.load())
        {
            int ch = get_key_nonblocking();
            if (ch == -1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }

            if (ch == 'q' || ch == 'Q')
            {
                stop_flag.store(true);
                break;
            }

            if (ch >= '1' && ch <= '9')
            {
                int key = ch - '0';
                if (active_key == key)
                {
                    release_key(key);
                    key_register.store(0);
                    //std::cout << "Released key " << key << " from gpio_state.\n";
                }
                else
                {
                    press_key(key);
                    key_register.store(key);
                    //std::cout << "Pressed key " << key << " and stored it in internal register.\n";
                }
            }
        }
    }
};

VirtualGPIO gpio;

bool VirtualGPIO_get(int key)
{
    static const std::map<uint8_t, uint8_t> button_pins_in =
    {
        {4, 0},
        {5, 1},
        {6, 2},
        {7, 3},
        {8, 4},
        {9, 5},
        {10, 6},
        {11, 7},
        {12, 8},
        {13, 9}
    };
    return gpio.get_state(button_pins_in.at(key));
}

int main()
{
    std::cout << "UniversalButtonMatrix host build (x86)\n";


    ButtonState bt_set{};

    while (!gpio.is_stopped())
    {
        if (gpio.has_key())
        {
            int key = gpio.get_key();
            //std::cout << "Internal key register contains: " << key << "\n";
            gpio.reset_key();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        button_board_handle(bt_set);
    }

    std::cout << "Exiting host test.\n";
    return 0;
}
