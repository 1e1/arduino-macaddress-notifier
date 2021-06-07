#ifndef WebServer_H_
#define WebServer_H_



extern "C" {
  #include "c_types.h"
}

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#if WS_WEB_SERVER_SECURE == WS_WEB_SERVER_SECURE_YES
#include <ESP8266WebServerSecure.h>
#include "certificate-generated.h"
#endif
#include <ArduinoJson.h>
#include <FS.h>
#include <list>
#include "config.h"
#include "macro.h"
#include "scm-generated.h"



class WebServer {

  public:
  WebServer(FS &fs);

  void begin(void) const;
  void loop(void) const;

  void setAuthentication(const String username, const String password);
  
  protected:
  void _setRoutes(void) const;

  const bool _isAllowed(void) const;
  void _streamAbout(void) const;
  void _streamHtml(const char* path) const;
  void _streamJson(const char* path, const char* defaultValue) const;
  void _uploadJson(const char* path) const;
  const size_t _getFileContents(const char* path, char* &buffer) const;

  char* _username;
  char* _password;

  #if WS_WEB_SERVER_SECURE == WS_WEB_SERVER_SECURE_YES
  BearSSL::ESP8266WebServerSecure* _server;
  #else
  ESP8266WebServer* _server;
  #endif
  FS* _fs;
};



#endif
