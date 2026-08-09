/*
 * The MIT License (MIT)
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
#include "bsp/board_api.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include <stdarg.h> 

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

#if defined(VARIANT_32_BUTTONS_ENABLED)
#define MODULE_NAME "Universal Button Matrix (32B)"
#elif defined(VARIANT_64_BUTTONS_ENABLED)
#define MODULE_NAME "Universal Button Matrix (64B)"
#elif defined(VARIANT_128_BUTTONS_ENABLED)
#define MODULE_NAME "Universal Button Matrix (128B)"
#else
#define MODULE_NAME "Universal Button Matrix"
#endif
//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

//---------------------------------------------------------
// HID REPORT
// (max. 128 buttons, 16 axis)
//---------------------------------------------------------

uint8_t const desc_hid_report[] =
{
    // ============================================================
    // Gamepad
    // ============================================================

    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,        // USAGE (Gamepad)
    0xA1, 0x01,        // COLLECTION (Application)

    0x85, REPORT_ID_GAMEPAD,    // REPORT_ID

    // ============================================================
    // 128 Buttons
    // ============================================================

    0x05, 0x09,        // USAGE_PAGE (Button)
    0x19, 0x01,        // USAGE_MINIMUM (Button 1)
    0x29, BUTTON_HID_REPORT_COUNT,        // USAGE_MAXIMUM (Button N)

    0x15, 0x00,        // LOGICAL_MINIMUM (0)
    0x25, 0x01,        // LOGICAL_MAXIMUM (1)

    0x75, 0x01,        // REPORT_SIZE (1)
    0x95, BUTTON_HID_REPORT_COUNT,        // REPORT_COUNT (N)

    0x81, 0x02,        // INPUT (Data,Var,Abs)


    // ============================================================
    // Axes
    // ============================================================

#if defined(SUPPORT_3_AXIS) || \
    defined(SUPPORT_9_AXIS) || \
    defined(SUPPORT_16_AXIS)

    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)

#if defined(SUPPORT_3_AXIS) || \
    defined(SUPPORT_9_AXIS) || \
    defined(SUPPORT_16_AXIS)

    0x09, 0x30,        // X
    0x09, 0x31,        // Y
    0x09, 0x32,        // Z

#endif

#if defined(SUPPORT_9_AXIS) || defined(SUPPORT_16_AXIS)

    0x09, 0x33,        // Rx
    0x09, 0x34,        // Ry
    0x09, 0x35,        // Rz
    0x09, 0x36,        // Slider
    0x09, 0x37,        // Dial
    0x09, 0x38,        // Wheel

#endif

#if defined(SUPPORT_16_AXIS)

    // ------------------------------------------------------------
    // Additional 7 axes
    // Vendor Defined Usage Page
    // ------------------------------------------------------------

    0x06, 0x00, 0xFF,  // USAGE_PAGE (Vendor Defined 0)

    0x09, 0x01,        // Axis 10
    0x09, 0x02,        // Axis 11
    0x09, 0x03,        // Axis 12
    0x09, 0x04,        // Axis 13
    0x09, 0x05,        // Axis 14
    0x09, 0x06,        // Axis 15
    0x09, 0x07,        // Axis 16

#endif

    // ------------------------------------------------------------
    // Axis data
    // ------------------------------------------------------------

    0x15, 0x81,        // LOGICAL_MINIMUM (-127)
    0x25, 0x7F,        // LOGICAL_MAXIMUM (127)

    0x75, 0x08,        // REPORT_SIZE (8)

#if defined(SUPPORT_3_AXIS)

    0x95, 0x03,        // REPORT_COUNT (3)

#elif defined(SUPPORT_9_AXIS)

    0x95, 0x09,        // REPORT_COUNT (9)

#elif defined(SUPPORT_16_AXIS)

    0x95, 0x10,        // REPORT_COUNT (16)

#endif

    0x81, 0x02,        // INPUT (Data,Var,Abs)

#endif // AXIS SUPPORT

    // ============================================================
    // End Gamepad
    // ============================================================

    0xC0               // END_COLLECTION
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance)
{
  (void) instance;
  return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
enum
{
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_HID,

    ITF_NUM_TOTAL
};
#else
enum
{
    ITF_NUM_HID = 0,

    ITF_NUM_TOTAL
};
#endif

#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
#define CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)

#else
#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#endif

#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_OUT     0x02
#define EPNUM_CDC_IN      0x82

#define EPNUM_HID         0x83
#else
#define EPNUM_HID         0x81
#endif

uint8_t const desc_configuration[] =
{
    TUD_CONFIG_DESCRIPTOR(
        1,
        ITF_NUM_TOTAL,
        0,
        CONFIG_TOTAL_LEN,
        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
        100),
#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
    TUD_CDC_DESCRIPTOR(
        ITF_NUM_CDC,
        4,
        EPNUM_CDC_NOTIF,
        8,
        EPNUM_CDC_OUT,
        EPNUM_CDC_IN,
        64),
#endif
    TUD_HID_DESCRIPTOR(
        ITF_NUM_HID,
        0,
        HID_ITF_PROTOCOL_NONE,
        sizeof(desc_hid_report),
        EPNUM_HID,
        CFG_TUD_HID_EP_BUFSIZE,
        5),

};

#if TUD_OPT_HIGH_SPEED
// Per USB specs: high speed capable device must report device_qualifier and other_speed_configuration

// other speed configuration
uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

// device qualifier is mostly similar to device descriptor since we don't change configuration based on speed
tusb_desc_device_qualifier_t const desc_device_qualifier =
{
  .bLength            = sizeof(tusb_desc_device_qualifier_t),
  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
  .bcdUSB             = USB_BCD,

  .bDeviceClass       = 0x00,
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,

  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
  .bNumConfigurations = 0x01,
  .bReserved          = 0x00
};

// Invoked when received GET DEVICE QUALIFIER DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete.
// device_qualifier descriptor describes information about a high-speed capable device that would
// change if the device were operating at the other speed. If not highspeed capable stall this request.
uint8_t const* tud_descriptor_device_qualifier_cb(void)
{
  return (uint8_t const*) &desc_device_qualifier;
}

// Invoked when received GET OTHER SEED CONFIGURATION DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
// Configuration descriptor in the other speed e.g if high speed then this is for full speed and vice versa
uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  // other speed config is basically configuration with type = OHER_SPEED_CONFIG
  memcpy(desc_other_speed_config, desc_configuration, CONFIG_TOTAL_LEN);
  desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

  // this example use the same configuration for both high and full speed mode
  return desc_other_speed_config;
}

#endif // highspeed

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  // This example use the same configuration for both high and full speed mode
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

// array of pointer to string descriptors
char const *string_desc_arr[] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "sysprog.pl",                  // 1: Manufacturer
  MODULE_NAME,                   // 2: Product
  NULL,                          // 3: Serials will use unique ID if possible
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void) langid;
  size_t chr_count;

  switch ( index ) {
    case STRID_LANGID:
      memcpy(&_desc_str[1], string_desc_arr[0], 2);
      chr_count = 1;
      break;

    case STRID_SERIAL:
      chr_count = board_usb_get_serial(_desc_str + 1, 32);
      break;

    default:
      // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
      // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

      if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) return NULL;

      const char *str = string_desc_arr[index];

      // Cap at max char
      chr_count = strlen(str);
      size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
      if ( chr_count > max_count ) chr_count = max_count;

      // Convert ASCII string into UTF-16
      for ( size_t i = 0; i < chr_count; i++ ) {
        _desc_str[1 + i] = str[i];
      }
      break;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}

void cdc_log_fmt(const char *fmt, ...)
{
#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
    if (!tud_cdc_connected())
        return;

    char buffer[256];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len <= 0)
        return;

    tud_cdc_write(buffer, len);
    tud_cdc_write_flush();
#endif
}

void cdc_log(const char *msg)
{
#if CONSOLE_DEBUG_VERBOSITY_LEVEL > 0
    tud_cdc_write(msg, strlen(msg));
    tud_cdc_write_flush();
#endif
}