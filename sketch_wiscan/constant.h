#ifndef constants_H_
#define constants_H_


/* mode */
#define WS_LOG_LEVEL_OFF            0
#define WS_LOG_LEVEL_FATAL          10
#define WS_LOG_LEVEL_ERROR          20
#define WS_LOG_LEVEL_WARN           30
#define WS_LOG_LEVEL_INFO           40
#define WS_LOG_LEVEL_DEBUG          50
#define WS_LOG_LEVEL_TRACE          60
#define WS_LOG_LEVEL_ALL            70


#define WS_CONFIG_KEY_PATH          "_KEY.txt"
#define WS_CONFIG_CERT_PATH         "_CSR.txt"
#define WS_CONFIG_GLOBAL_PATH       "global.json"
#define WS_CONFIG_WIFI_PATH         "wifis.json"
#define WS_CONFIG_DEVICE_PATH       "relays.json"
#define WS_CONFIG_RULE_PATH         "rules.json"
#define WS_CONFIG_TRANSPORT_PATH    "transport.json"
#define WS_WEB_INDEX_BASENAME       "index"
#define WS_WEB_PORTAL_BASENAME      "portal"


#define WS_WEB_PORT_DEFAULT         80
#define WS_WEB_PORT_DEFAULT_SECURE  443
#define WS_WEB_SERVER_SECURE_NO     0
#define WS_WEB_SERVER_SECURE_YES    1
#define WS_WEB_FILE_EXT_BR          "br"
#define WS_WEB_FILE_EXT_GZ          "gz"

#define WS_CONFIG_BUFFER_SIZE       1024
#define WS_RULE_PIN_NONE            ((uint8_t) -1)
 

#endif // constants_H_
