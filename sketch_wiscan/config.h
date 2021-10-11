#ifndef config_H_
#define config_H_

#include "_config.h"



// board metadata
// =========================
// a number between 2 and 253
#ifndef WS_PIN_SAFEMODE
#define WS_PIN_SAFEMODE     A0
#endif
#ifndef WS_PIN_CONFIG
#define WS_PIN_CONFIG       A0
#endif
#ifndef WS_LOG_LEVEL
#define WS_LOG_LEVEL        WS_LOG_LEVEL_ALL
#endif
// =========================


// Serial conf
// =========================
// multiply by the clock divisor (ex:9600 baud at 2MHz instead of 16Mhz is 1200 baud)
#ifndef WS_SERIAL    
#define WS_SERIAL           Serial
#endif
#ifndef WS_SERIAL_SPEED    
#define WS_SERIAL_SPEED     115200
#endif
#ifndef WS_USB_SPEED
#define WS_USB_SPEED        9600
#endif
// =========================


// WEB conf
// =========================
#ifndef WS_WEB_PORT
#define WS_WEB_PORT             WS_WEB_PORT_DEFAULT
#endif
#ifndef WS_WEB_SERVER_SECURE
#define WS_WEB_SERVER_SECURE    WS_WEB_SERVER_SECURE_NO
#endif
#ifndef WS_WEB_FILE_EXT
#define WS_WEB_FILE_EXT         WS_WEB_FILE_EXT_GZ
#endif
// =========================


// Wifi conf
// =========================
#ifndef WS_WIFI_SSID
#define WS_WIFI_SSID                    "HelloWorld"
#endif
#ifndef WS_WIFI_PASSWORD
//#define WS_WIFI_PASSWORD              "Open Sesame!"
#endif
#ifndef WS_WIFI_CONNEXION_TIMEOUT_MS
#define WS_WIFI_CONNEXION_TIMEOUT_MS    30000
#endif
#ifndef WS_WIFI_STA_OUTPUT_POWER
#define WS_WIFI_STA_OUTPUT_POWER        20
#endif
#ifndef WS_WIFI_AP_OUTPUT_POWER
#define WS_WIFI_AP_OUTPUT_POWER         10
#endif
#ifndef WS_WIFI_STA_PHY_MODE
#define WS_WIFI_STA_PHY_MODE            WIFI_PHY_MODE_11N
#endif
#ifndef WS_WIFI_AP_PHY_MODE
// WIFI_PHY_MODE_11B | WIFI_PHY_MODE_11G | WIFI_PHY_MODE_11N
#define WS_WIFI_AP_PHY_MODE             WIFI_PHY_MODE_11N
#endif
// =========================


// Wifi conf
// =========================
#ifndef WS_DEVICE_DETECTED_COUNTDOWN_S
#define WS_DEVICE_DETECTED_COUNTDOWN_S  180
#endif
// =========================



#endif // config_H_
