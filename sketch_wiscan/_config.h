#ifndef _config_H_
#define _config_H_

#include "constant.h"



// board metadata
// =========================
#define WS_PIN_SAFEMODE     A0 /* RXD */
#define WS_PIN_CONFIG       A0 /* GPIO12 */
#define WS_RELAY_NB_MAX     32
#define WS_LOG_LEVEL        WS_LOG_LEVEL_OFF
// =========================


// Serial conf
// =========================
// multiply by the clock divisor (ex:9600 baud at 2MHz instead of 16Mhz is 1200 baud)
#define WS_SERIAL_SPEED     115200
// =========================


// WEB conf
// =========================
#define WS_WEB_PORT             WS_WEB_PORT_DEFAULT_SECURE
#define WS_WEB_SERVER_SECURE    WS_WEB_SERVER_SECURE_YES
#define WS_WEB_FILE_EXT         WS_WEB_FILE_EXT_BR
// =========================



#endif // _config_H_
