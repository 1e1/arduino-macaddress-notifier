#include "Configuration.h"




/***********************************************************
 *                       PROPERTIES                        *
 **********************************************************/








/***********************************************************
 *                         PUBLIC                          *
 **********************************************************/




Configuration::Configuration(FS &fs)
{
  this->_fs = &fs;
}


void Configuration::begin()
{
  if (_fs->exists(WS_CONFIG_GLOBAL_PATH)) {
    this->_loadGlobal();
  } else {
    this->setSafeMode();
  }
}


void Configuration::setSafeMode(const bool isSafeMode)
{
  if (isSafeMode) {
    Configuration::Global g {
      .acl = {
        .username = "",
        .password = "",
        .timeout = WS_DEVICE_DETECTED_COUNTDOWN_S,
        .isSafeMode = true,
        .canAutoRestart = false,
      },
      .wifiAp = {
        .ssid = WS_WIFI_SSID,
        .password = {},
        .channel = 1,
        .isHidden = false,
      }
    };

    this->_global = g;
  }
}


const std::list<Configuration::WifiStation> Configuration::getWifiStationList() const
{
  std::list<Configuration::WifiStation> wifiStationList;
  if (_fs->exists(WS_CONFIG_WIFI_PATH)) {
    JsonArray root = this->_open(WS_CONFIG_WIFI_PATH)->as<JsonArray>();
    
    for (JsonObject o : root) {
      Configuration::WifiStation wifi {
        .ssid = o["n"].as<String>(),
        .password = o["p"].as<String>(),
      };

      wifiStationList.emplace_back(wifi);
    }
  }

  return wifiStationList;
}


const std::list<Configuration::Device> Configuration::getDeviceList() const
{
  std::list<Configuration::Device> deviceList;
  if (_fs->exists(WS_CONFIG_DEVICE_PATH)) {
    JsonArray root = this->_open(WS_CONFIG_DEVICE_PATH)->as<JsonArray>();

    for (JsonObject o : root) {
      Configuration::Device device {
        .id = o["i"].as<uint8_t>(),
        .mac = new char[strlen(o["m"])],
        .name = o["n"].as<String>(),
        .cmac = new uint8_t[6],
      };

      strcpy(device.mac, o["m"]);

      // device.cmac = ether_aton(device.mac);
      uint8_t cmacBuffer;
      uint8_t cmacIndex = 0;
      bool cmacIsUnit = false;
      for(char* it = device.mac; *it && cmacIndex < 6; ++it) {
        if (*it != ':') {
          uint8_t v = (*it >= 'A') ? (*it >= 'a') ? (*it - 'a' + 10) : (*it - 'A' + 10) : (*it - '0');
          if (cmacIsUnit) {
            device.cmac[cmacIndex++] = cmacBuffer | v;
            cmacIsUnit = false;
          } else {
            cmacBuffer = v << 4;
            cmacIsUnit = true;
          }
        }
      }
      
      deviceList.emplace_back(device);
    }
  }

  return deviceList;
}


const std::list<Configuration::Rule> Configuration::getRuleList() const
{
  std::list<Configuration::Rule> ruleList;
  if (_fs->exists(WS_CONFIG_RULE_PATH)) {
    JsonArray root = this->_open(WS_CONFIG_RULE_PATH)->as<JsonArray>();

    for (JsonObject o : root) {
      Configuration::Rule rule {
        .key = o["k"].as<String>(),
        .pin = o["p"].as<uint8_t>(),
        .equation = o["e"].as<String>(),
        .direction = static_cast<StateDirection>(o["d"].as<int>()),
      };
      
      ruleList.emplace_back(rule);
    }
  }

  return ruleList;
}


const Configuration::Transport Configuration::getTransport() const
{
  JsonObject root = this->_open(WS_CONFIG_TRANSPORT_PATH)->as<JsonObject>();
  
  Configuration::Transport t {
    .uri = root["u"].as<String>(),
    .method = root["m"].as<String>(),
    .payload = root["p"].as<String>(),
  };

  return t;
}



/***********************************************************
 *                        PROTECTED                        *
 **********************************************************/




JsonDocument* Configuration::_open(const char* filename) const
{
  File file = _fs->open(filename, "r"); // "w+"
  DynamicJsonDocument* doc = new DynamicJsonDocument(WS_CONFIG_BUFFER_SIZE);
  deserializeJson(*doc, file, DeserializationOption::NestingLimit(2));
  //ReadBufferingStream bufferingStream(file, 64);
  //DeserializationError error = deserializeMsgPack(doc, bufferingStream, DeserializationOption::NestingLimit(2));
  file.close();
  doc->shrinkToFit();

  /*
  if (error) {
    Serial.print("deserializeMsgPack() failed: ");
    Serial.println(error.f_str());
    return;
  }
  */

  return doc;
}


void Configuration::_loadGlobal()
{
  JsonObject root = this->_open(WS_CONFIG_GLOBAL_PATH)->as<JsonObject>();
  
  Configuration::Global g {
    .acl = {
      .username = root["u"].as<String>(),
      .password = root["w"].as<String>(),
      .timeout = root["t"].as<unsigned int>(),
      .isSafeMode = false,
      .canAutoRestart = root["r"].as<bool>(),
    },
    .wifiAp = {
      .ssid = root["n"].as<String>(),
      .password = root["p"].as<String>(),
      .channel = root["c"].as<uint8_t>(),
      .isHidden = root["h"].as<bool>(),
    }
  };

  this->_global = g;
}
