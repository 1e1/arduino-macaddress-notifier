#ifndef RpnSolver_H_
#define RpnSolver_H_



#include <ESP8266WiFi.h>
#include <functional>
#include <list>
#include "config.h"
#include "macro.h"



class RpnSolver {

  public:
  using valueOfId = int (*) (int id);
  //typedef int (* valueOfId)(int id);

  void addMapper(valueOfId mapper);
  const bool check(String equation) const;
  const int resolve(String equation);

  protected:
  void _push(int value);
  int _pop(void);
  void _compute1(char operand);
  void _compute2(char operand);
  void _compute3(char operand);

  valueOfId _mapper = nullptr;
  std::list<int> _stack;

  static const char _separators[];
  static const char _operators1[];
  static const char _operators2[];
  static const char _operators3[];
};



#endif
