#include "WebServer.h"




/***********************************************************
 *                       PROPERTIES                        *
 **********************************************************/



#if WM_WEB_SERVER_SECURE == WM_WEB_SERVER_SECURE_YES
static BearSSL::ServerSessions _serverCache(WS_WEB_SERVER_CACHE_SIZE);
#endif

static constexpr const char TEXT_HTML[] PROGMEM = "text/html";
static constexpr const char TEXT_JSON[] PROGMEM = "text/json";
static constexpr const char PLAIN[] PROGMEM = "plain";
static constexpr const char CONTENT_ENCODING[] PROGMEM = "Content-Encoding";
static constexpr const char CACHE_CONTROL[] PROGMEM = "Cache-Control";
static constexpr const char X_HEAP[] PROGMEM = "X-Heap";
static constexpr const char X_UPTIME[] PROGMEM = "X-Uptime";
static constexpr const char MAX_AGE_86400[] PROGMEM = "max-age=86400";

static constexpr const char _jsonArrayEmpty[] = "[]";
static constexpr const char _jsonObjectEmpty[] = "{}";





/***********************************************************
 *                         PUBLIC                          *
 **********************************************************/




WebServer::WebServer(FS &fs)
{
  #if WS_WEB_SERVER_SECURE == WS_WEB_SERVER_SECURE_YES
  this->_server = new BearSSL::ESP8266WebServerSecure(WS_WEB_PORT);
  
  LOG(F("certificate "));
  this->_server->getServer().setCache(&_serverCache);

  if (certificate::serverCertType == certificate::CertType::CT_ECC) {
    LOGLN(F("ECC"));
    this->_server->getServer().setECCert(new BearSSL::X509List(certificate::serverCert), BR_KEYTYPE_KEYX|BR_KEYTYPE_SIGN, new BearSSL::PrivateKey(certificate::serverKey));
  } else if(certificate::serverCertType == certificate::CertType::CT_RSA) {
    LOGLN(F("RSA"));
    this->_server->getServer().setRSACert(new BearSSL::X509List(certificate::serverCert), new BearSSL::PrivateKey(certificate::serverKey));
  } else {
    LOGLN(F("ERROR"));
  }
  #else
  this->_server = new ESP8266WebServer(WS_WEB_PORT);
  #endif

  this->_fs = &fs;

  this->_username = new char[1]; // { '\0' };
  this->_password = new char[1]; // { '\0' };
} 


void WebServer::begin(void) const
{
  this->_setRoutes();

  //this->_server->getServer().setNoDelay(true);
  this->_server->begin();
}


void WebServer::loop(void) const
{
  this->_server->handleClient();
}


void WebServer::setAuthentication(String username, String password)
{
  this->_username = new char[username.length() + 1];
  strcpy(this->_username, username.c_str());

  this->_password = new char[password.length() + 1];
  strcpy(this->_password, password.c_str());

  LOG("'"); LOG(_username); LOG("' : '"); LOG(_password); LOGLN("'");
}



/***********************************************************
 *                        PROTECTED                        *
 **********************************************************/




void WebServer::_setRoutes(void) const
{
  this->_server->on("/", HTTP_GET, [=]() {
    this->_streamHtml(WS_WEB_INDEX_BASENAME "." WS_WEB_FILE_EXT);
  });

  this->_server->on("/portal", HTTP_GET, [=]() {
    if (this->_isAllowed()) {
      this->_streamHtml(WS_WEB_PORTAL_BASENAME "." WS_WEB_FILE_EXT);
    }
  });

  this->_server->on("/cfg/g", HTTP_GET, [=]() {
    this->_streamJson(WS_CONFIG_GLOBAL_PATH, _jsonObjectEmpty);
  });

  this->_server->on("/cfg/g", HTTP_POST, [=]() {
    this->_uploadJson(WS_CONFIG_GLOBAL_PATH);
  });

  this->_server->on("/cfg/w", HTTP_GET, [=]() { 
    this->_streamJson(WS_CONFIG_WIFI_PATH, _jsonArrayEmpty);
  });

  this->_server->on("/cfg/w", HTTP_POST, [=]() {
    this->_uploadJson(WS_CONFIG_WIFI_PATH);
  });

  this->_server->on("/cfg/d", HTTP_GET, [=]() { 
    this->_streamJson(WS_CONFIG_DEVICE_PATH, _jsonArrayEmpty);
  });

  this->_server->on("/cfg/d", HTTP_POST, [=]() {
    this->_uploadJson(WS_CONFIG_DEVICE_PATH);
  });

  this->_server->on("/cfg/r", HTTP_GET, [=]() { 
    this->_streamJson(WS_CONFIG_RULE_PATH, _jsonArrayEmpty);
  });

  this->_server->on("/cfg/r", HTTP_POST, [=]() {
    this->_uploadJson(WS_CONFIG_RULE_PATH);
  });

  this->_server->on("/cfg/t", HTTP_GET, [=]() { 
    this->_streamJson(WS_CONFIG_TRANSPORT_PATH, _jsonObjectEmpty);
  });

  this->_server->on("/cfg/t", HTTP_POST, [=]() {
    this->_uploadJson(WS_CONFIG_TRANSPORT_PATH);
  });
  
  this->_server->on("/cfg/reboot", HTTP_DELETE, [=]() {
    if (this->_isAllowed()) {
      this->_server->send(200, FPSTR(TEXT_JSON), F("\"reboot\""));
      LOGLN(F("** RESTART **"));
      ESP.restart();
    }
  });
  
  this->_server->on("/about", HTTP_GET, [=]() {
    this->_streamAbout();
  });
}


const bool WebServer::_isAllowed(void) const
{
  if (this->_username[0] != '\0' && this->_password[0] != '\0') {
    if (!this->_server->authenticate(this->_username, this->_password)) {
      this->_server->requestAuthentication();

      return false;
    }
  }

  return true;
}


void WebServer::_streamAbout(void) const
{
  this->_server->sendHeader(FPSTR(X_HEAP), String(ESP.getFreeHeap()));
  this->_server->sendHeader(FPSTR(X_UPTIME), String(millis()));
  this->_server->send(
    200, 
    FPSTR(TEXT_JSON), 
    F("{\"repo\":\"" SCM_REPO "\",\"hash\":\"" SCM_HASH "\",\"date\":\"" SCM_DATE "\",\"chan\":\"" SCM_CHAN "\"}")
  );
}


void WebServer::_streamHtml(const char* path) const
{
  this->_server->sendHeader(FPSTR(CONTENT_ENCODING), F(WM_WEB_FILE_EXT));
  this->_server->sendHeader(FPSTR(CACHE_CONTROL), FPSTR(MAX_AGE_86400));

  File file = this->_fs->open(path, "r");
  this->_server->streamFile(file, FPSTR(TEXT_HTML));
  file.close();
}


void WebServer::_streamJson(const char* path, const char* defaultValue) const
{
  if (this->_isAllowed()) {
    if (!this->_fs->exists(path)) {
      this->_server->send(200, FPSTR(TEXT_JSON), defaultValue);
    }

    File file = this->_fs->open(path, "r");
    this->_server->streamFile(file, FPSTR(TEXT_JSON));
    file.close();
  }
}


void WebServer::_uploadJson(const char* path) const
{
  if (this->_isAllowed()) {
    if (!this->_server->hasArg(FPSTR(PLAIN))) {
      return this->_server->send(400/*, FPSTR(TEXT_JSON), _jsonObjectEmpty*/); // TODO
    }

    String payload = this->_server->arg(FPSTR(PLAIN));
    DynamicJsonDocument doc(WS_CONFIG_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::NestingLimit(2));
    LOGLN(payload);
    
    if (!error) {
      File file = this->_fs->open(path, "w");
      serializeJson(doc, file);
      //serializeMsgPack(doc, file);
      file.close();
    }

    this->_streamJson(path, "null");
  }
}


const size_t WebServer::_getFileContents(const char* path, char* &buffer) const
{
  File file = this->_fs->open(path, "r");
  const size_t size = file.size();
  std::unique_ptr<char[]> buf(new char[size]);
  file.readBytes(buf.get(), size);
  file.close();

  return size;
}
