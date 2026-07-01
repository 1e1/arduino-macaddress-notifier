#ifndef HOST_SHIM_ESP8266WIFI_H
#define HOST_SHIM_ESP8266WIFI_H

/**
 * Minimal host-side shim to build RpnSolver off-device.
 * RpnSolver only needs the String type and isDigit() from the Arduino core,
 * so that is all this provides. Do not use for anything else.
 */

#include <string>
#include <cstring>
#include <cctype>
#include <stdint.h>

class String {
  public:
    String() {}
    String(const char* s) : _s(s ? s : "") {}
    String(const std::string& s) : _s(s) {}

    unsigned int length() const { return (unsigned int) _s.size(); }
    char charAt(unsigned int i) const { return _s[i]; }
    String& operator+=(char c) { _s.push_back(c); return *this; }
    const char* c_str() const { return _s.c_str(); }

  private:
    std::string _s;
};

inline bool isDigit(char c) { return std::isdigit((unsigned char) c) != 0; }

#endif // HOST_SHIM_ESP8266WIFI_H
