#ifndef _BLE_RCSP_SERVER_H_
#define _BLE_RCSP_SERVER_H_

static const uint8_t profile_data[] = {
    //////////////////////////////////////////////////////
    //
    // 0x0001 PRIMARY_SERVICE  1800
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,

    /* CHARACTERISTIC,  2a00, READ | WRITE | DYNAMIC, */
    // 0x0002 CHARACTERISTIC 2a00 READ | WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x0a, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE 2a00 READ | WRITE | DYNAMIC
    0x08, 0x00, 0x0a, 0x01, 0x03, 0x00, 0x00, 0x2a,

#if 1
    //////////////////////////////////////////////////////
    //
    // 0x0004 PRIMARY_SERVICE  ae00
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x80, 0x00, 0x00, 0x28, 0x00, 0xae,

    /* CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0040 CHARACTERISTIC ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x81, 0x00, 0x03, 0x28, 0x04, 0x82, 0x00, 0x01, 0xae,
    // 0x0041 VALUE ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x82, 0x00, 0x01, 0xae,

    /* CHARACTERISTIC,  ae02, NOTIFY, */
    // 0x0042 CHARACTERISTIC ae02 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x83, 0x00, 0x03, 0x28, 0x10, 0x84, 0x00, 0x02, 0xae,
    // 0x0043 VALUE ae02 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x84, 0x00, 0x02, 0xae,
    // 0x0044 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x85, 0x00, 0x02, 0x29, 0x00, 0x00,
#else
    //////////////////////////////////////////////////////
    //
    // 0x0007 PRIMARY_SERVICE 		66F0
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x28, 0xf0, 0x66,

    /* CHARACTERISTIC,		66F1,	WRITE_WITHOUT_RESPONSE |  DYNAMIC */
    // 0x0008 CHARACTERISTIC 66F1 WRITE_WITHOUT_RESPONSE |  DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x08, 0x00, 0x03, 0x28, 0x04, 0x09, 0x00, 0xf1, 0x66,
    // 0x0009 VALUE 66F1 WRITE_WITHOUT_RESPONSE |  DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x09, 0x00, 0xf1, 0x66,

    /* CHARACTERISTIC,		66F2,	NOTIFY | DYNAMIC */
    // 0x000a CHARACTERISTIC 66F2 NOTIFY | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x03, 0x28, 0x10, 0x0b, 0x00, 0xf2, 0x66,
    // 0x000b VALUE 66F2 NOTIFY | DYNAMIC
    0x08, 0x00, 0x10, 0x01, 0x0b, 0x00, 0xf2, 0x66,
    // 0x000c CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x0c, 0x00, 0x02, 0x29, 0x00, 0x00,
#endif
    // END
    0x00, 0x00,
};
//
// characteristics <--> handles
//
#if 1
#define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0082
#define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x0084
#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x0085
#else
#define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0009
#define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x000c
#endif

#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003

#endif





