#ifndef macro_H_
#define macro_H_

#include "config.h"



/** DEBUGGING TOOLS **/
#if WS_LOG_LEVEL != WS_LOG_LEVEL_OFF
  #define LOG_START() Serial.begin(WS_USB_SPEED)
  #define LOG(...)    Serial.print  (__VA_ARGS__)
  #define LOGLN(...)  Serial.println(__VA_ARGS__); Serial.flush()
  #define WAIT(ms)    delay(ms)
#else
  #define LOG_START()
  #define LOG(...)
  #define LOGLN(...)
  #define WAIT(ms)
#endif
/** === **/



#if defined(LED_BUILTIN) && !defined(WS_LOG_LED)
  #define WS_LOG_LED LED_BUILTIN
#endif

#ifdef WS_LOG_LED
  #define BUSYLED_INIT pinMode(WS_LOG_LED, OUTPUT)
  #define BUSYLED_ON digitalWrite(WS_LOG_LED, LOW)
  #define BUSYLED_OFF digitalWrite(WS_LOG_LED, HIGH)
#else
  #define BUSYLED_INIT
  #define BUSYLED_ON
  #define BUSYLED_OFF
#endif



/** FAST TRIGO **/
#define FACTOR_PI_UINT12(x) byte((50*x) >> 4)
#define FACTOR_PI_UINT8(x)  byte((804*x) >> 8)
/** === **/



/** LOOP **/
// wikipedia "foreach_loop"
#define FOREACH(idxtype, idxpvar, col, colsiz)  idxtype* idxpvar; for( idxpvar=col ; idxpvar < (col + (colsiz)) ; idxpvar++)
#define ARRAYLEN(ary)                           ( sizeof(ary)/sizeof(*ary) )
#define FILLARRAY(a,n)                          a[0]=n, memcpy( ((char*)a)+sizeof(a[0]), a, sizeof(a)-sizeof(a[0]) );
#define FAST_STRCMP(x, y)   (*(x) != *(y) ? \
                              ((unsigned char) *(x) - (unsigned char) *(y)) : \
                              strcmp((x), (y)))
#define FAST_STRCMP_P(x, y) (*(x) != *(y) ? \
                              ((unsigned char) *(x) - (unsigned char) *(y)) : \
                              strcmp_P((x), (y)))
/*
FOREACH (char, p1, c1, strlen(c1) ) {
  printf("loop 1 : %c\n",*p1);
}
FOREACH (int, p2, c2, ARRAYLEN(c2) ){
  printf("loop 2 : %d\n",*p2);
}
*/
/** === **/



/** BIT **/
#define bitRead_uint8_t(value, bit)             byte(((value) >> (bit)) & byte(1))
#define bitSet_uint8_t(value, bit)              ((value) |=  (byte(1) << (bit)))
#define bitClear_uint8_t(value, bit)            ((value) &= ~(byte(1) << (bit)))
#define bitWrite_uint8_t(value, bit, bitvalue)  (bitvalue ? bitSet_uint8_t(value, bit) : bitClear_uint8_t(value, bit))

#define bitRead_boolean(value, bit)             byte(((value) >> (bit)) & byte(1))
#define bitSet_boolean(value, bit)              ((value) |=  (byte(1) << (bit)))
#define bitClear_boolean(value, bit)            ((value) &= ~(byte(1) << (bit)))
#define bitWrite_boolean(value, bit, bitvalue)  (bitvalue ? bitSet_boolean(value, bit) : bitClear_boolean(value, bit))
/** === **/



#define STR_HELPER(...) #__VA_ARGS__
#define STR(...)        STR_HELPER(__VA_ARGS__)

#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0


#define SOFTWARE_RESET    asm volatile ("jmp 0")


#endif // macro_H_
