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



#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
//#include <fauxmoESP.h>
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
//fauxmoESP fauxmo;
std::list<Configuration::Rule> ruleList;
std::list<Configuration::Device> deviceList;
Configuration::Transport transport;

uint8_t* countdownDetectedDeviceList;
int* previousValueRuleList;
bool* ackRuleList;
uint8_t countdown_s = WS_DEVICE_DETECTED_COUNTDOWN_S;


void setup()
{
  BUSYLED_INIT;
  BUSYLED_ON;
  LOG_START();
  WAIT(1000);
  LOGLN(PSTR("DEBUG ON"));

  LOG("reset reason: "); LOGLN(ESP.getResetReason());
  
  /**
   * init
   */
  LittleFS.begin();
  configuration = new Configuration(LittleFS);

  pinMode(WS_PIN_CONFIG, INPUT_PULLUP);
  pinMode(WS_PIN_SAFEMODE, INPUT_PULLUP);

  bool isUnlocked = digitalRead(WS_PIN_CONFIG) == HIGH;
  bool isSafeMode = digitalRead(WS_PIN_SAFEMODE) == HIGH;
  bool externalReset = ESP.getResetInfoPtr()->reason == rst_reason::REASON_EXT_SYS_RST;

  LOG("isUnlocked="); LOGLN(isUnlocked);
  LOG("isSafeMode="); LOGLN(isSafeMode);
  LOG("externalReset="); LOGLN(externalReset);

  LOGLN(PSTR("-- load Configuration"));
  configuration->begin();
  configuration->setSafeMode(isSafeMode);

  if (isUnlocked || externalReset) {
    configuration->getGlobal()->acl.canAutoRestart = false;
  }
  LOGLN(PSTR("---"));

  if (externalReset) {
    startWiFi();

    WebServer* server;

    // start portal

    LOGLN(PSTR("-- setup WebServer"));
    WebServer::setFs(LittleFS);
    WebServer::setAuthentication(configuration->getGlobal()->acl.username, configuration->getGlobal()->acl.password);

    server = new WebServer();
    server->begin();

    LOGLN(PSTR("---"));

    LOGLN(PSTR("-- setup mDNS"));
    MDNS.begin(certificate::dname);
    MDNS.addService(WS_WEB_SERVER_SECURE == WS_WEB_SERVER_SECURE_YES ? "https" : "http", "tcp", WS_WEB_PORT);
    LOGLN(PSTR("---"));

    BUSYLED_ON;
    // infinite loop
    do {
      if (configuration->getGlobal()->acl.canAutoRestart) {
        if (WiFi.status() != WL_CONNECTED) {
          BUSYLED_OFF;
          LOGLN(PSTR("** RESTART **"));

          ESP.deepSleepInstant(30E6, WAKE_RF_DISABLED); // TODO constantize microsecond
          ESP.restart();
        }
      }

      server->loop();
      MDNS.update();
    } while (true);
  }

  LOGLN(PSTR("-- init lists"));
  uint8_t index = 0;
  deviceList = configuration->getDeviceList();
  ruleList = configuration->getRuleList();
  transport = configuration->getTransport();

  countdownDetectedDeviceList = new uint8_t[deviceList.size()];
  previousValueRuleList = new int[ruleList.size()];
  ackRuleList = new bool[ruleList.size()];
  LOGLN(PSTR("---"));

  LOGLN(PSTR("-- init pins"));
  index = 0;
  for (Configuration::Device device : deviceList) {
    countdownDetectedDeviceList[index] = 0;
    ++index;
  }
  LOGLN();

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
  LOGLN();
  LOGLN(PSTR("---"));

  LOGLN(PSTR("-- init features"));
  esppl_init(parseFrame);
  fastTimer = new FastTimer(FastTimer::P_1s_4m);
  rpnSolver = new RpnSolver();
  rpnSolver->addMapper(hasDetectedDeviceById);
  LOGLN(PSTR("---"));

  esppl_sniffing_start();

  BUSYLED_OFF;
}


void startWiFi(void)
{
  WiFi.hostname(certificate::dname);

  if (!configuration->getGlobal()->acl.isSafeMode) {
    BUSYLED_ON;
    LOGLN(PSTR("-- trying to connect to STA:"));

    /* */
    WiFi.mode(WIFI_STA);
    std::list<Configuration::WifiStation> wifiList = configuration->getWifiStationList();
    for (Configuration::WifiStation wifi : wifiList) {
      LOGLN(wifi.ssid);
      WiFi.begin(wifi.ssid, wifi.password);
      
      if (WiFi.waitForConnectResult(WS_WIFI_CONNEXION_TIMEOUT_MS) == WL_CONNECTED) {
        break;
      }
    }
    /* * /
    ESP8266WiFiMulti wifiMulti;
    std::list<Configuration::WifiStation> wifiList = configuration->getWifiStationList();
    for (Configuration::WifiStation wifi : wifiList) {
      LOGLN(wifi.ssid);
      wifiMulti.addAP(wifi.ssid, wifi.password);
    }

    if (wifiMulti.run(WS_WIFI_CONNEXION_TIMEOUT_MS) == WL_CONNECTED) {
      LOG(PSTR("connected at: ")); LOGLN(WiFi.SSID());
    }
    /* */
    LOGLN(PSTR("---"));
  }

  if (WiFi.status() != WL_CONNECTED) {
    /**
     * set mode Guardian
     * (Configuration*, Configuration::Global*)
     */
    LOGLN(PSTR("-- trying to create AP:"));
    LOG(PSTR("AP ssid: "));LOGLN(configuration->getGlobal()->wifiAp.ssid);
    LOG(PSTR("AP password: "));LOGLN(configuration->getGlobal()->wifiAp.password);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(
      configuration->getGlobal()->wifiAp.ssid, 
      configuration->getGlobal()->wifiAp.password, 
      configuration->getGlobal()->wifiAp.channel, 
      configuration->getGlobal()->wifiAp.isHidden
    );

    LOGLN(PSTR("---"));

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


void loop()
{
  bool hasUpdate = false;
  int index;

  if (fastTimer->update()) {
    BUSYLED_OFF;

    if (countdown_s > 0) {
      --countdown_s;
      LOG("countdown_s="); LOGLN(countdown_s);
    }

    index = 0;
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
  
  for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
    esppl_set_channel(i);
    for (uint8_t loopi=0; loopi<5; loopi++) {
      while (esppl_process_frames()) {
        //
      }
    }
  }

  index = 0;
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
      ackRuleList[index] = false;
    }

    hasUpdate |= !ackRuleList[index];

    ++index;
  }

  if (hasUpdate && (countdown_s == 0)) {
    esppl_sniffing_stop();
    wifi_promiscuous_enable(false);

    startWiFi();
    BUSYLED_ON;
    
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    HTTPClient https;

    char* method = new char[transport.method.length() + 1];
    strcpy(method, transport.method.c_str());

    index = 0;
    for (Configuration::Rule rule : ruleList) {
      if (!ackRuleList[index]) {
        if (rule.pin != WS_RULE_PIN_NONE) {
          digitalWrite(rule.pin, previousValueRuleList[index]>0 ? HIGH : LOW);
        }

        if (rule.key.length() == 0) {
          ackRuleList[index] = true;
        } else {
          String value = String(previousValueRuleList[index]);

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
        }
      }

      ++index;
    }

    esppl_init(parseFrame);
    esppl_sniffing_start();
  } 
}
