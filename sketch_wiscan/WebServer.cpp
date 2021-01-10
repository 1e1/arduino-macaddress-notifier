#include "WebServer.h"




/***********************************************************
 *                       PROPERTIES                        *
 **********************************************************/



#if WS_WEB_SERVER_SECURE == WS_WEB_SERVER_SECURE_YES
static BearSSL::ESP8266WebServerSecure _server(WS_WEB_PORT);
//static char* _serverKey;
//static char* _serverCert;

#else
static ESP8266WebServer _server(WS_WEB_PORT);
#endif


static fs::FS* _fs = nullptr;
static char* _username = new char[1]; // { '\0' };
static char* _password = new char[1]; // { '\0' };





/***********************************************************
 *                         PUBLIC                          *
 **********************************************************/




void WebServer::begin()
{
  this->_setup();
}


void WebServer::loop()
{
  _server.handleClient();
}


void WebServer::setAuthentication(String username, String password)
{
  _username = new char[username.length() + 1];
  strcpy(_username, username.c_str());

  _password = new char[password.length() + 1];
  strcpy(_password, password.c_str());

  LOG("'"); LOG(_username); LOG("' : '"); LOG(_password); LOGLN("'");
}


void WebServer::setFs(FS &fs)
{
  _fs = &fs;
}



/***********************************************************
 *                        PROTECTED                        *
 **********************************************************/




void WebServer::_setup()
{
  _server.on("/", HTTP_GET, []() {
    WebServer::_streamHtml(WS_WEB_INDEX_BASENAME "." WS_WEB_FILE_EXT, true);
  });

  _server.on("/portal", HTTP_GET, []() {
    WebServer::_streamHtml(WS_WEB_PORTAL_BASENAME "." WS_WEB_FILE_EXT, false);
  });

  _server.on("/cfg/g", HTTP_GET, []() {
    WebServer::_streamJson(WS_CONFIG_GLOBAL_PATH, "{}", false);
  });

  _server.on("/cfg/g", HTTP_POST, []() {
    WebServer::_uploadJson(WS_CONFIG_GLOBAL_PATH);
  });

  _server.on("/cfg/w", HTTP_GET, []() { 
    WebServer::_streamJson(WS_CONFIG_WIFI_PATH, "[]", false);
  });

  _server.on("/cfg/w", HTTP_POST, []() {
    WebServer::_uploadJson(WS_CONFIG_WIFI_PATH);
  });

  _server.on("/cfg/d", HTTP_GET, []() { 
    WebServer::_streamJson(WS_CONFIG_DEVICE_PATH, "[]", false);
  });

  _server.on("/cfg/d", HTTP_POST, []() {
    WebServer::_uploadJson(WS_CONFIG_DEVICE_PATH);
  });

  _server.on("/cfg/r", HTTP_GET, []() { 
    WebServer::_streamJson(WS_CONFIG_RULE_PATH, "[]", false);
  });

  _server.on("/cfg/r", HTTP_POST, []() {
    WebServer::_uploadJson(WS_CONFIG_RULE_PATH);
  });

  _server.on("/cfg/t", HTTP_GET, []() { 
    WebServer::_streamJson(WS_CONFIG_TRANSPORT_PATH, "{}", false);
  });

  _server.on("/cfg/t", HTTP_POST, []() {
    WebServer::_uploadJson(WS_CONFIG_TRANSPORT_PATH);
  });
  
  _server.on("/cfg/reboot", HTTP_DELETE, []() {
    if (WebServer::_isAllowed()) {
      _server.send(200, "text/json", "\"reboot\"");
      LOGLN(F("** RESTART **"));
      ESP.restart();
    }
  });
  
  _server.on("/about", HTTP_GET, []() {
    _server.send(200, "text/json", "{\"hash\":\"" SCM_HASH "\",\"date\":\"" SCM_DATE "\",\"chan\":\"" SCM_CHAN "\"}");
  });

  #if WS_WEB_SERVER_SECURE == WS_WEB_SERVER_SECURE_YES
  LOG(F("certificate "));
  //WebServer::_getFileContents(WS_CONFIG_KEY_PATH, _serverKey);
  //WebServer::_getFileContents(WS_CONFIG_CERT_PATH, _serverCert);

  if (certificate::serverCertType == certificate::CertType::CT_ECC) {
    LOGLN(F("ECC"));
    _server.getServer().setECCert(new BearSSL::X509List(certificate::serverCert), BR_KEYTYPE_KEYX|BR_KEYTYPE_SIGN, new BearSSL::PrivateKey(certificate::serverKey));
  } else if(certificate::serverCertType == certificate::CertType::CT_RSA) {
    LOGLN(F("RSA"));
    _server.getServer().setRSACert(new BearSSL::X509List(certificate::serverCert), new BearSSL::PrivateKey(certificate::serverKey));
  } else {
    LOGLN(F("ERROR"));
  }
  #endif

  _server.begin();
}


const bool WebServer::_isAllowed()
{
  if (_username[0] != '\0' && _password[0] != '\0') {
    if (!_server.authenticate(_username, _password)) {
      _server.requestAuthentication();

      return false;
    }
  }

  return true;
}


void WebServer::_streamHtml(const char* path, const bool isPublic)
{
  if (isPublic || WebServer::_isAllowed()) {
    _server.sendHeader(String(F("Content-Encoding")), String(F(WS_WEB_FILE_EXT)));
    _server.sendHeader(String(F("Cache-Control")), String(F("max-age=86400")));

    File file = _fs->open(path, "r");
    _server.streamFile(file, "text/html");
    file.close();
  }
}


void WebServer::_streamJson(const char* path, const char* defaultValue, const bool isPublic)
{
  if (isPublic || WebServer::_isAllowed()) {
    if (_fs->exists(path)) {
      File file = _fs->open(path, "r");
      _server.streamFile(file, "text/json");
      file.close();

      return;
    }
  }

  _server.send(200, "text/json", defaultValue);
}


void WebServer::_uploadJson(const char* path)
{
  if (WebServer::_isAllowed()) {
    if (_server.hasArg("plain")) {
      String payload = _server.arg("plain");
      DynamicJsonDocument doc(WS_CONFIG_BUFFER_SIZE);
      DeserializationError error = deserializeJson(doc, payload, DeserializationOption::NestingLimit(2));
      LOGLN(payload);
      
      if (!error) {
        File file = _fs->open(path, "w");
        serializeJson(doc, file);
        //serializeMsgPack(doc, file);
        file.close();
      }
    }

    WebServer::_streamJson(path, "null");
  }
}


const size_t WebServer::_getFileContents(const char* path, char* &buffer)
{
  File file = _fs->open(path, "r");
  const size_t size = file.size();
  std::unique_ptr<char[]> buf(new char[size]);
  file.readBytes(buf.get(), size);
  file.close();

  return size;
}
