#include "FastTimer.h"




/***********************************************************
 *                       PROPERTIES                        *
 **********************************************************/








/***********************************************************
 *                         PUBLIC                          *
 **********************************************************/




FastTimer::FastTimer(const Precision precision)
{
  this->_precision = precision;
  this->_section = 0;
  this->_embedTime = -1; // force "new time section" at startup
}




/**
  *   0: same time section
  *   1: new time section
  * 255: new time section, new cycle
  *
  * @return int8_t
  */
const uint8_t FastTimer::update()
{
  const uint8_t previousTime = FastTimer::_embedTime;
  FastTimer::_embedTime = byte(millis() >> this->_precision);
  FastTimer::_section = FastTimer::_embedTime ^ previousTime;
  return FastTimer::_section;
}
