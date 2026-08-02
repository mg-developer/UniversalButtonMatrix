#include <stdio.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"


#pragma pack(push,1)
struct GamepadReport
{
    uint8_t buttons[16];   // 128 buttons
};
#pragma pack(pop)


GamepadReport report;


void set_button(uint8_t button, bool pressed)
{
    if (button < 1 || button > 128)
        return;

    button--;

    uint8_t byte = button / 8;
    uint8_t bit  = button % 8;

    if (pressed)
        report.buttons[byte] |= (1 << bit);
    else
        report.buttons[byte] &= ~(1 << bit);
}

bool Buttons[128] = {};

void BuildReport()
{
    memset(&report, 0, sizeof(report));

    for (int i = 0; i < 128; i++)
    {
        if (Buttons[i])
        {
            report.buttons[i / 8] |= (1 << (i % 8));
        }
    }
}

uint8_t currentButton = 0;
bool pressed = true;

uint32_t lastChange = board_millis();

int main()
{
    board_init();

    tusb_init();
    
    stdio_init_all();


    // Example button on GPIO2
    gpio_init(2);
    gpio_set_dir(2, GPIO_IN);
    gpio_pull_up(2);


    while (true)
    {
        tud_task();


        uint32_t now = board_millis();

        if (now - lastChange >= 500)
        {
            lastChange = now;

            if (pressed)
            {
                Buttons[currentButton] = false;
                pressed = false;
            }
            else
            {
                currentButton = (currentButton + 1) % 128;

                Buttons[currentButton] = true;
                pressed = true;
            }
        }


        if (tud_hid_ready())
        {
            BuildReport();

            tud_hid_report(
                0,
                &report,
                sizeof(report)
            );
        }

    sleep_ms(1);
    }
}
