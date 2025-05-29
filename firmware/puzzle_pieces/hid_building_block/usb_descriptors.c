#include "tusb.h"
#include "class/hid/hid.h"

// Helper macros
#ifndef LOBYTE
#define LOBYTE(x)  ((uint8_t)((x) & 0x00FFU))
#endif
#ifndef HIBYTE  
#define HIBYTE(x)  ((uint8_t)(((x) & 0xFF00U) >> 8U))
#endif

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    
    .idVendor           = 0x2E8A, // Raspberry Pi
    .idProduct          = 0x0003, // Raspberry Pi Pico
    .bcdDevice          = 0x0100,
    
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    
    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
uint8_t const desc_hid_report[] = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,        // Usage (0x01)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (0x01)
    0x15, 0x80,        //   Logical Minimum (-128)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x01,        //   Usage (0x01)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x09, 0x01,        //   Usage (0x01)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};

// Invoked when received GET HID REPORT DESCRIPTOR
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
enum {
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (9 + 9 + 9 + 7)  // Config + Interface + HID + Endpoint
#define EPNUM_HID           0x81

uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    9,                                          // bLength
    TUSB_DESC_CONFIGURATION,                    // bDescriptorType
    LOBYTE(CONFIG_TOTAL_LEN), HIBYTE(CONFIG_TOTAL_LEN), // wTotalLength
    1,                                          // bNumInterfaces
    1,                                          // bConfigurationValue
    0,                                          // iConfiguration
    TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,        // bmAttributes
    50,                                         // bMaxPower (100mA)

    // Interface Descriptor
    9,                                          // bLength
    TUSB_DESC_INTERFACE,                        // bDescriptorType
    ITF_NUM_HID,                               // bInterfaceNumber
    0,                                          // bAlternateSetting
    1,                                          // bNumEndpoints
    TUSB_CLASS_HID,                            // bInterfaceClass
    0,                                          // bInterfaceSubClass
    0,                                          // bInterfaceProtocol
    0,                                          // iInterface

    // HID Descriptor
    9,                                          // bLength
    HID_DESC_TYPE_HID,                         // bDescriptorType
    LOBYTE(0x0111), HIBYTE(0x0111),            // bcdHID
    0,                                          // bCountryCode
    1,                                          // bNumDescriptors
    HID_DESC_TYPE_REPORT,                      // bDescriptorType
    LOBYTE(sizeof(desc_hid_report)), HIBYTE(sizeof(desc_hid_report)), // wDescriptorLength

    // Endpoint Descriptor
    7,                                          // bLength
    TUSB_DESC_ENDPOINT,                        // bDescriptorType
    EPNUM_HID,                                 // bEndpointAddress
    TUSB_XFER_INTERRUPT,                       // bmAttributes
    LOBYTE(CFG_TUD_HID_EP_BUFSIZE), HIBYTE(CFG_TUD_HID_EP_BUFSIZE), // wMaxPacketSize
    5                                           // bInterval
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
char const* string_desc_arr [] = {
    (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
    "Raspberry Pi",                // 1: Manufacturer
    "Pico HID Interface",          // 2: Product
    "123456",                      // 3: Serials, should use chip ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;

    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        // Convert ASCII string into UTF-16
        if (!(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0]))) return NULL;

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        for(uint8_t i=0; i<chr_count; i++) {
            _desc_str[1+i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

    return _desc_str;
}