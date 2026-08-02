#include "tusb.h"


//---------------------------------------------------------
// HID REPORT
// 128 buttons
//---------------------------------------------------------

uint8_t const hid_report_descriptor[] =
{
    0x05,0x01,        // Usage Page Generic Desktop
    0x09,0x05,        // Usage Game Pad

    0xA1,0x01,        // Collection


    0x05,0x09,        // Usage Page Button

    0x19,0x01,        // Button 1
    0x29,0x80,        // Button 128


    0x15,0x00,
    0x25,0x01,


    0x75,0x01,        // 1 bit
    0x95,0x80,        // 128 fields


    0x81,0x02,        // Input


    0xC0
};



extern "C"
{


//---------------------------------------------------------
// Device descriptor
//---------------------------------------------------------

uint8_t const desc_device[] =
{
    18,
    TUSB_DESC_DEVICE,

    0x00,0x02,

    0x00,
    0x00,
    0x00,

    64,


    // VID
    0x09,0x12,

    // PID
    0x01,0x00,


    0x00,0x01,


    1,
    2,
    3,


    1
};



uint8_t const* tud_descriptor_device_cb(void)
{
    return desc_device;
}



//---------------------------------------------------------
// Configuration descriptor
//---------------------------------------------------------

uint8_t const desc_configuration[] =
{

    // Configuration descriptor

    9,
    TUSB_DESC_CONFIGURATION,

    34,0,

    1,

    1,

    0,

    0x80,

    50,



    // Interface

    9,
    TUSB_DESC_INTERFACE,

    0,

    0,

    1,

    TUSB_CLASS_HID,

    0,

    0,

    0,



    // HID descriptor

    9,
    HID_DESC_TYPE_HID,

    0x11,0x01,

    0,

    1,

    HID_DESC_TYPE_REPORT,

    sizeof(hid_report_descriptor),0,



    // Endpoint

    7,

    TUSB_DESC_ENDPOINT,

    0x81,

    TUSB_XFER_INTERRUPT,

    64,0,

    10

};



uint8_t const* tud_descriptor_configuration_cb(
    uint8_t index)
{
    (void)index;

    return desc_configuration;
}



//---------------------------------------------------------
// HID descriptor callback
//---------------------------------------------------------

uint8_t const* tud_hid_descriptor_report_cb(
    uint8_t instance)
{
    (void)instance;

    return hid_report_descriptor;
}



//---------------------------------------------------------
// Strings
//---------------------------------------------------------

char const* strings[] =
{
    "",
    "MG Controls",
    "128 Button Controller",
    "0001"
};



uint16_t const* tud_descriptor_string_cb(
    uint8_t index,
    uint16_t langid)
{

    static uint16_t buffer[32];


    if(index==0)
    {
        buffer[0]=(TUSB_DESC_STRING<<8)|4;

        buffer[1]=0x0409;

        return buffer;
    }


    if(index>=4)
        return nullptr;


    char const* str=strings[index];


    uint8_t len=strlen(str);


    for(uint8_t i=0;i<len;i++)
        buffer[1+i]=str[i];


    buffer[0]=
        (TUSB_DESC_STRING<<8)
        |
        (2*len+2);


    return buffer;
}



//---------------------------------------------------------
// HID callbacks
//---------------------------------------------------------

uint16_t tud_hid_get_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t* buffer,
    uint16_t reqlen)
{
    return 0;
}



void tud_hid_set_report_cb(
    uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const* buffer,
    uint16_t bufsize)
{

}


}
