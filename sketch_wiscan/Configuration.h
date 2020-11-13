#ifndef Configuration_h
#define Configuration_h



#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <limits.h>
#include <list>
// #include <netinet/ether.h>
#include "config.h"
#include "macro.h"



class Configuration {

  public:
  typedef enum { S_change=-1, S_fall=0, S_raise=1 } StateDirection;

  struct Acl {
    String username;
    String password;
    uint timeout;
    bool isSafeMode;
    bool canAutoRestart;
  };

  struct WifiAp {
    String ssid;
    String password;
    uint8_t channel: 4;
    bool isHidden;
  };

  struct Global {
    Acl acl;
    WifiAp wifiAp;
  };

  struct WifiStation {
    String ssid;
    String password;
  };

  struct Device {
    uint8_t id;
    char* mac;
    String name;
    uint8_t* cmac;
  };

  struct Rule {
    String key;
    uint8_t pin;
    String equation;
    StateDirection direction;
  };

  struct Transport {
    String uri;
    String method;
    String payload;
  };

  Configuration(FS &fs);

  void begin(void);
  void setSafeMode(const bool isSafeMode=true);
  Global* getGlobal(void) { return &this->_global; };
  const std::list<WifiStation> getWifiStationList(void);
  const std::list<Device> getDeviceList(void);
  const std::list<Rule> getRuleList(void);
  const Transport getTransport(void);

  protected:
  fs::FS* _fs = nullptr;
  Global _global;

  DynamicJsonDocument _open(const char* filename);
  void _loadGlobal(void);

};



#endif
