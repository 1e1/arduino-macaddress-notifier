#ifndef FastTimer_H_
#define FastTimer_H_

#include <Arduino.h>
#include "macro.h"


/** TIME **/
//==>  >> 0: 1 unit of embedTime is 0.001s
//-->  const unsigned long maxTime        = 4294967295; // = 49d 17h 02m 47s
//-->  const unsigned int maxTime         = 65535;      // = 65s
//==>  >> 10: 1 unit of embedTime is 1.024s
//-->  const uint8_t maxEmbedTime         = 255;        // = 4mn 21s 120ms
//==>  >> 12: 1 unit of embedTime is 4.096s
//-->  const unsigned int maxEmbedTime    = 65535;      // = 3d 02h 33mn 51s 360ms
//     const unsigned int moduloEmbedTime = 63281;      // = 3d 00h 00mn 00s 000ms
//-->  const uint8_t maxEmbedTime         = 255;        // = 17mn 24s 480ms
//     const uint8_t moduloEmbedTime      = 219;        // = 15mn 00s 000ms
//==>  >> 14: 1 unit of embedTime is 16.384s
//-->  const uint8_t maxEmbedTime         = 255;        // = 1h 09mn 37s 920ms
//     const uint8_t moduloEmbedTime      = 219;        // = 1h 00mn 00s 000ms
//==>  >> 16: 1 unit of embedTime is 65.536s
//-->  const uint8_t maxEmbedTime         = 255;        // = 4h 38mn 31s 680ms
//     const uint8_t moduloEmbedTime      = 219;        // = 4h 00mn 00s 000ms

//#define WS_FASTTIMER_PRECISION_1s_4m    byte(millis() >> 10)
//#define WS_FASTTIMER_PRECISION_4s_15m   byte(millis() >> 12)
//#define WS_FASTTIMER_PRECISION_4s_3d    int(millis() >> 12)
//#define WS_FASTTIMER_PRECISION_16s_1h   byte(millis() >> 14)
//#define WS_FASTTIMER_PRECISION_65s_4h   byte(millis() >> 16)


class FastTimer {

  public:
  typedef enum { P_1s_4m=10, P_4s_15m=12, P_16s_1h=14, P_65s_4h=16 } Precision;

  FastTimer(const Precision precision);

  const uint8_t update(void); // call it once in the main loop()

  // inline
  __attribute__((always_inline)) inline const boolean isSection(const byte section) const { return bitRead_boolean(this->_section, section); };
  __attribute__((always_inline)) inline const uint8_t getTime(void) const { return this->_embedTime; };

  protected:
  Precision _precision;
  uint8_t _section;
  uint8_t _embedTime;

};


#endif
