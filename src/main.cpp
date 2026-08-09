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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shiftregister.h"
#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

static_assert(VARIANT_32_BUTTONS_ENABLED + VARIANT_64_BUTTONS_ENABLED + VARIANT_128_BUTTONS_ENABLED == 1, "Exactly one button variant must be enabled");

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);
void cdc_task(void);
void button_task(void);
void set_button(uint8_t button, bool pressed);
void cdc_log_print(const char *msg);

bool Buttons[128] = {};

#pragma pack(push,1)
struct GamepadReport
{
    uint8_t buttons[16];

#if defined(SUPPORT_3_AXIS) || \
    defined(SUPPORT_9_AXIS) || \
    defined(SUPPORT_16_AXIS)
    int8_t axes[EnabledAxisSupportCount];
#endif
};
#pragma pack(pop)

static_assert(sizeof(GamepadReport) == 16 + (EnabledAxisSupportCount));

struct GamepadReport report;


/*------------- MAIN -------------*/
int main()
{
  board_init();

  // init device stack on configured roothub port
  const tusb_rhport_init_t rh_init = {
    .role = TUSB_ROLE_DEVICE,
    .speed = TUD_OPT_HIGH_SPEED ? TUSB_SPEED_HIGH : TUSB_SPEED_FULL
  };
  TU_ASSERT(tud_rhport_init(BOARD_TUD_RHPORT, &rh_init));
  board_init_after_tusb();

  button_board_init();

  static bool startup_logged = false;
  static bool connected = false;
  static absolute_time_t connected_time;
  int logline = 0;
  while (1)
  {
      tud_task();

  #if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
    if (tud_cdc_connected())
    {
        if (!connected)
        {
            connected = true;
            connected_time = get_absolute_time();
        }

        if (!startup_logged &&
            absolute_time_diff_us(connected_time, get_absolute_time()) > 500000)
        {
            cdc_log_fmt("%d: == Configured ROWxCOL [%dx%d], Axis count:%d ==\r\n",
                      logline++,
                      BUTTON_INPUT_ROWS,
                      BUTTON_OUTPUT_COLUMNS,
                      EnabledAxisSupportCount);

            startup_logged = true;
        }
    }
    else
    {
        connected = false;
    }

  #endif
    
      hid_task();
      sleep_us(100);
  }

}

bool fill_report(GamepadReport& dst_gr, const ButtonState& src_state)
{
    //std::memset(dst_gr.buttons, 0, sizeof(dst_gr.buttons));
    bool some_key_pressed = false;

    for (size_t i = 0; i < src_state.size() && i < 128; ++i)
    {
        if (src_state[i])
        {
            dst_gr.buttons[i / 8] |= static_cast<uint8_t>(1u << (i % 8));
            some_key_pressed = true;
        }
    }
    return some_key_pressed;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb()
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb()
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb()
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, const GamepadReport &gr_source)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      memset(&report, 0, sizeof(report));
      // Deep copy
      report = gr_source;

      tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
      has_gamepad_key = true;

    }
    break;

    default: break;
  }
}

#if TUSB_VERSION_NUMBER > 1800
// board_millis has been removed from tinyusb. Use tusb_time_millis_api instead
#define board_millis tusb_time_millis_api
#endif

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  ButtonState bt_set;
  button_board_handle(bt_set);

  struct GamepadReport gr_source = {};
  bool some_button_pressed = fill_report(gr_source, bt_set);

  // Remote wakeup
  if ( tud_suspended() && some_button_pressed )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_GAMEPAD, gr_source);
  }
}

#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void) 
{
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_connected() )
  {
    // connected and there are data available
    if (tud_cdc_available()) {
      // read data
      char buf[64];
      uint32_t count = tud_cdc_read(buf, sizeof(buf));
      (void) count;

      // Echo back
      // Note: Skip echo by commenting out write() and write_flush()
      // for throughput test e.g
      //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
      tud_cdc_write(buf, count);
      tud_cdc_write_flush();
    }
  }
}
#endif


// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if (dtr) {
    // Terminal connected
  } else {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) {
  (void) itf;
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  ButtonState bt_set;
  button_board_handle(bt_set);

  struct GamepadReport gr_source = {};
  fill_report(gr_source, bt_set);

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, gr_source);
  }
}

static GamepadReport g_gamepad_report;

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer,
                               uint16_t reqlen)
{
    (void)instance;

    if (report_type != HID_REPORT_TYPE_INPUT)
        return 0;

    if (report_id != REPORT_ID_GAMEPAD)
        return 0;

    uint16_t len = sizeof(GamepadReport);

    if (len > reqlen)
        len = reqlen;

    memcpy(buffer, &g_gamepad_report, len);

    return len;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK (DISABLED)
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

void set_button(uint8_t button, bool pressed)
{
    if (button < 1 || button > 128)
        return;

    button--;

    uint8_t byte = button / 8;
    uint8_t bit  = button % 8;

    if (pressed)
        Buttons[byte] |= (1 << bit);
    else
        Buttons[byte] &= ~(1 << bit);
}

