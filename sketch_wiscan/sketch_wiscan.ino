/**
 * setup
 * =====
 * if external reset
 * then STA/softAP + portal
 * else scan
 * 
 * loop
 * ====
 * scan WiFi
 * if friends
 * then
 *   connect STA
 *   
 */



#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <sys/time.h>
#include <time.h>
#include "config.h"
#include "macro.h"
#include "certificate-generated.h"
#include "Configuration.h"
#include "Fasttimer.h"
#include "WebServer.h"
#include "RpnSolver.h"
#include "FriendDetector/esppl_functions.h"



/** ===================== **/




Configuration* configuration;
FastTimer* fastTimer;
RpnSolver* rpnSolver;
std::list<Configuration::Rule> ruleList;
std::list<Configuration::Device> deviceList;
Configuration::Transport transport;

uint8_t* countdownDetectedDeviceList;
int* previousValueRuleList;
bool* ackRuleList;
uint8_t countdown_s;


void setup()
{
  BUSYLED_INIT;
  BUSYLED_ON;
  LOG_START();
  WAIT(1000);
  LOGLN(F("DEBUG ON"));

  LOG("reset reason: "); LOGLN(ESP.getResetReason());
  
  /**
   * init
   */
  LittleFS.begin();
  configuration = new Configuration(LittleFS);

  pinMode(WS_PIN_CONFIG, INPUT_PULLUP);
  pinMode(WS_PIN_SAFEMODE, INPUT_PULLUP);

  const bool isUnlocked = digitalRead(WS_PIN_CONFIG) == HIGH;
  const bool isSafeMode = digitalRead(WS_PIN_SAFEMODE) == HIGH;
  const bool externalReset = ESP.getResetInfoPtr()->reason == rst_reason::REASON_EXT_SYS_RST;

  LOG(F("isUnlocked=")); LOGLN(isUnlocked);
  LOG(F("isSafeMode=")); LOGLN(isSafeMode);
  LOG(F("externalReset=")); LOGLN(externalReset);

  LOGLN(F("-- load Configuration"));
  configuration->begin();
  configuration->setSafeMode(isSafeMode);

  if (isUnlocked || externalReset) {
    configuration->getGlobal()->acl.canAutoRestart = false;
  }

  LOG(F("acl.username: ")); LOGLN(configuration->getGlobal()->acl.username);
  LOG(F("acl.timeout: ")); LOGLN(configuration->getGlobal()->acl.timeout);
  LOG(F("acl.isSafeMode: ")); LOGLN(configuration->getGlobal()->acl.isSafeMode);
  LOG(F("acl.canAutoRestart: ")); LOGLN(configuration->getGlobal()->acl.canAutoRestart);
  LOG(F("wifiAp.ssid: ")); LOGLN(configuration->getGlobal()->wifiAp.ssid);
  LOG(F("wifiAp.channel: ")); LOGLN(configuration->getGlobal()->wifiAp.channel);
  LOG(F("wifiAp.isHidden: ")); LOGLN(configuration->getGlobal()->wifiAp.isHidden);
  LOGLN(F("---"));

  if (externalReset) {
    startWiFi();

    WebServer* server;

    // start portal

    LOGLN(F("-- setup WebServer"));
    server = new WebServer(LittleFS);
    server->setAuthentication(configuration->getGlobal()->acl.username, configuration->getGlobal()->acl.password);
    server->begin();
    LOGLN(F("---"));

    LOGLN(F("-- setup mDNS"));
    MDNS.begin(certificate::dname);
    MDNS.addService(WS_WEB_SERVER_SECURE == WS_WEB_SERVER_SECURE_YES ? "https" : "http", "tcp", WS_WEB_PORT);
    LOGLN(F("---"));

    BUSYLED_ON;
    // infinite loop
    do {
      if (configuration->getGlobal()->acl.canAutoRestart) {
        if (WiFi.status() != WL_CONNECTED) {
          BUSYLED_OFF;
          LOGLN(F("** RESTART **"));

          ESP.deepSleepInstant(ESP.deepSleepMax(), WAKE_RF_DISABLED);
          ESP.restart();
        }
      }

      server->loop();
      yield();
      MDNS.update();
      yield();
    } while (true);
  }

  LOGLN(F("-- init globals"));
  countdown_s = configuration->getGlobal()->acl.timeout;
  LOGLN(F("---"));

  LOGLN(F("-- init lists"));
  uint8_t index = 0;
  deviceList = configuration->getDeviceList();
  ruleList = configuration->getRuleList();
  transport = configuration->getTransport();

  countdownDetectedDeviceList = new uint8_t[deviceList.size()];
  previousValueRuleList = new int[ruleList.size()];
  ackRuleList = new bool[ruleList.size()];
  LOGLN(F("---"));

  LOGLN(F("-- init pins"));
  index = 0;
  for (Configuration::Device device : deviceList) {
    countdownDetectedDeviceList[index] = 0;
    ++index;
  }
  LOG(F("nbDevices=")); LOGLN(index);

  index = 0;
  for (Configuration::Rule rule : ruleList) {
    // WARNING check: https://github.com/esp8266/Arduino/blob/74819a763bfb6e9890a57411dcea4aba221a778d/variants/d1_mini/pins_arduino.h
    if (rule.pin != WS_RULE_PIN_NONE) {
      pinMode(rule.pin, OUTPUT);
    }

    previousValueRuleList[index] = INT_MIN;
    ackRuleList[index] = false;
    ++index;
  }
  LOG(F("nbRules=")); LOGLN(index);
  LOGLN(F("---"));

  LOGLN(F("-- init features"));
  esppl_init(parseFrame);
  fastTimer = new FastTimer(FastTimer::P_1s_4m);
  rpnSolver = new RpnSolver();
  rpnSolver->addMapper(hasDetectedDeviceById);
  LOGLN(F("---"));

  esppl_sniffing_start();

  BUSYLED_OFF;
}


void startWiFi(void)
{
  if (!configuration->getGlobal()->acl.isSafeMode) {
    BUSYLED_ON;
    LOGLN(F("-- trying to connect to STA:"));
    WiFi.mode(WIFI_STA);
    WiFi.hostname(certificate::dname);
    WiFi.setOutputPower(WS_WIFI_STA_OUTPUT_POWER);
    WiFi.setPhyMode(WS_WIFI_STA_PHY_MODE);

    ESP8266WiFiMulti wifiMulti;
    std::list<Configuration::WifiStation> wifiList = configuration->getWifiStationList();
    for (Configuration::WifiStation wifi : wifiList) {
      LOGLN(wifi.ssid);
      wifiMulti.addAP(wifi.ssid.c_str() + '\0', wifi.password.c_str() + '\0');
    }

    if (wifiMulti.run(WS_WIFI_CONNEXION_TIMEOUT_MS) == WL_CONNECTED) {
      LOG(F("connected at: ")); LOGLN(WiFi.SSID());
    }
    /* */
    LOGLN(F("---"));
  }

  if (WiFi.status() != WL_CONNECTED) {
    /**
     * set mode Guardian
     * (Configuration*, Configuration::Global*)
     */
    LOGLN(F("-- trying to create AP:"));
    WiFi.mode(WIFI_AP);
    WiFi.hostname(certificate::dname);
    WiFi.setOutputPower(WS_WIFI_AP_OUTPUT_POWER);
    WiFi.setPhyMode(WS_WIFI_AP_PHY_MODE);

    LOG(F("AP ssid: "));LOGLN(configuration->getGlobal()->wifiAp.ssid);
    LOG(F("AP password: "));LOGLN(configuration->getGlobal()->wifiAp.password);

    //IPAddress myIp(192, 168, 0, 1); // TODO CONSTANTIZE
    //WiFi.softAPConfig(myIp, myIp, IPAddress(255, 255, 255, 0));

    WiFi.softAP(
      configuration->getGlobal()->wifiAp.ssid, 
      configuration->getGlobal()->wifiAp.password, 
      configuration->getGlobal()->wifiAp.channel, 
      configuration->getGlobal()->wifiAp.isHidden
    );

    LOGLN(F("---"));

    BUSYLED_OFF;
  }
}


int hasDetectedDeviceById(int id) {
  int index = 0;

  for (Configuration::Device device : deviceList) {
    if (id == device.id) {
      return (int) (countdownDetectedDeviceList[index] > 0);
    }

    ++index;
  }

  return 0;
}


bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}


void parseFrame(esppl_frame_info *info) {
  int index = 0;

  for (Configuration::Device device : deviceList) {
    if (maccmp(info->sourceaddr, device.cmac) || maccmp(info->receiveraddr, device.cmac)) {
      if (countdownDetectedDeviceList[index] == 0) {
        LOG(device.name); LOGLN(" detected");
      }

      countdownDetectedDeviceList[index] = configuration->getGlobal()->acl.timeout;
    }

    ++index;
  }
}


void scanWifi() {
  for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
    esppl_set_channel(i);
    for (uint8_t loopi=0; loopi<5; loopi++) {
      while (esppl_process_frames()) {
        //
      }
    }
  }
}


void countdownDetectedDevices() {
  uint8_t index = 0;

  for (Configuration::Device device : deviceList) {
    if (countdownDetectedDeviceList[index] > 0) {
      if (countdownDetectedDeviceList[index] == 0) {
        LOG(device.name); LOGLN(" has left");
      }

      countdownDetectedDeviceList[index] = countdownDetectedDeviceList[index] - 1;
    }

    ++index;
  }
}


const bool hasUpdatedResults() {
  bool hasUpdate = false;
  uint8_t index = 0;

  for (Configuration::Rule rule : ruleList) {
    const int value = rpnSolver->resolve(rule.equation);
    
    if (value != previousValueRuleList[index]) {
      bool update = false;

      switch (rule.direction) {
        case Configuration::StateDirection::S_change:
          update = value != previousValueRuleList[index];
          break;
        case Configuration::StateDirection::S_raise:
          update = value > previousValueRuleList[index];
          break;
        case Configuration::StateDirection::S_fall:
          update = value < previousValueRuleList[index];
          break;
      }

      previousValueRuleList[index] = value;
      ackRuleList[index] &= !update;
    }

    hasUpdate |= !ackRuleList[index];

    ++index;
  }

  return hasUpdate;
}


void loop()
{
  if (fastTimer->update()) {
    BUSYLED_OFF;
    LOGF("[HW] Free heap: %d bytes\n", ESP.getFreeHeap());

    if (countdown_s > 0) {
      --countdown_s;
      LOG("countdown_s="); LOGLN(countdown_s);
    }

    countdownDetectedDevices();
  }
  
  scanWifi();

  if ((countdown_s == 0) && hasUpdatedResults()) {

    if (transport.uri.length() == 0) {
      uint8_t index = 0;
      for (Configuration::Rule rule : ruleList) {
        if (!ackRuleList[index]) {
          if (rule.pin != WS_RULE_PIN_NONE) {
            digitalWrite(rule.pin, previousValueRuleList[index]>0 ? HIGH : LOW);
          }
          
          ackRuleList[index] = true;
        }

        ++index;
      }
    } else {
      /* prepare HTTPClient */
      esppl_sniffing_stop();
      wifi_promiscuous_enable(false);

      startWiFi();
      BUSYLED_ON;
      
      std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
      client->setInsecure();

      HTTPClient https;

      char* method = new char[transport.method.length() + 1];
      strcpy(method, transport.method.c_str());
      /* ------------------ */

      uint8_t index = 0;
      for (Configuration::Rule rule : ruleList) {
        if (!ackRuleList[index]) {
          if (rule.pin != WS_RULE_PIN_NONE) {
            digitalWrite(rule.pin, previousValueRuleList[index]>0 ? HIGH : LOW);
          }

          if (rule.key.length() == 0) {
            ackRuleList[index] = true;
          } else {
            /* use HTTPClient */
            //String value = String(previousValueRuleList[index]);
            char value[4];
            itoa(previousValueRuleList[index], value, 10);

            String uri = String(transport.uri);
            uri.replace("{key}", rule.key);
            uri.replace("{value}", value);

            String payload = String(transport.payload);
            payload.replace("{key}", rule.key);
            payload.replace("{value}", value);

            LOG(transport.method); LOG(' '); LOGLN(uri);
            LOGLN(payload);

            if (https.begin(*client, uri)) {
              int httpCode = https.sendRequest(method, payload);
              LOGLN(httpCode);

              ackRuleList[index] = httpCode == HTTP_CODE_OK;
            }
            /* -------------- */
          }
        }

        ++index;
      }

      esppl_init(parseFrame);
      esppl_sniffing_start();
    }
  } 
}
