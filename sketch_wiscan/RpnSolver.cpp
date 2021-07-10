#include "RpnSolver.h"




/***********************************************************
 *                       PROPERTIES                        *
 **********************************************************/



constexpr const char RpnSolver::_separators[] = " ";
constexpr const char RpnSolver::_operators1[] = "!";
constexpr const char RpnSolver::_operators2[] = "+-*/<=>";
constexpr const char RpnSolver::_operators3[] = "?";




/***********************************************************
 *                         PUBLIC                          *
 **********************************************************/




void RpnSolver::addMapper(valueOfId mapper)
{
  this->_mapper = mapper;
}


const bool RpnSolver::check(String equation) const
{
  //LOG("eq? "); LOGLN(equation);

  uint8_t stackSize = 0;
  bool readingDigit = false;
  bool requireDigit = false;

  const uint8_t equationLength = equation.length();
  char c;
  for(uint8_t i=0; i<equationLength; i++) {
    c = equation.charAt(i);

    if (requireDigit) {
      if (isDigit(c)) {
        readingDigit = true;
        requireDigit = false;
      } else {
        return false;
      }
    } else {
      if (isDigit(c)) {
        if (!readingDigit) {
          ++stackSize;
          readingDigit = true;
        }
      } else if ('@'==c) {
        ++stackSize;
        requireDigit = true;
      } else if (strrchr(RpnSolver::_operators2, c)) {
        --stackSize;
      } else if (strrchr(RpnSolver::_operators1, c)) {

      } else if (strrchr(RpnSolver::_operators3, c)) {
        stackSize = stackSize - 2;
      } else if (strrchr(RpnSolver::_separators, c)) {
        readingDigit = false;
      } else {
        return false;
      }
    }
  }

  return (requireDigit == false) && (stackSize == 1);
}


const int RpnSolver::resolve(String equation)
{
  //LOG("eq= "); LOGLN(equation);

  equation += RpnSolver::_separators[0];

  int digitBuffer = 0;
  bool hasDigitBuffer = false;
  bool isReference = false;

  this->_stack.clear();
  
  const uint8_t equationLength = equation.length();
  char c;
  for(uint8_t i=0; i<equationLength; i++) {
    c = equation.charAt(i);

    if (isDigit(c)) {
      digitBuffer = (digitBuffer * 10) + (c - '0');
      hasDigitBuffer = true;
    } else {
      if (hasDigitBuffer) {
        hasDigitBuffer = false;

        if (isReference) {
          isReference = false;
          digitBuffer = (this->_mapper)(digitBuffer);
        }
        
        this->_push(digitBuffer);

        digitBuffer = 0;
      } else {
        if (isReference) {
          LOG("unknown reference in equation");
        }
      }

      if ('@'==c) {
        isReference = true;
      } else if (strrchr(RpnSolver::_operators2, c)) {
        this->_compute2(c);
      } else if (strrchr(RpnSolver::_operators1, c)) {
        this->_compute1(c);
      } else if (strrchr(RpnSolver::_operators3, c)) {
        this->_compute3(c);
      } else if (strrchr(RpnSolver::_separators, c)) {
        //
      } else {
        LOG("unknown token '"); LOG(c); LOGLN("' in equation");
      }
    }
  }

  return this->_stack.back();
}



/***********************************************************
 *                        PROTECTED                        *
 **********************************************************/




void RpnSolver::_push(int value)
{
  this->_stack.emplace_back(value);
}

int RpnSolver::_pop(void)
{
  int out = this->_stack.back();
  this->_stack.pop_back();

  return out;
}

void RpnSolver::_compute1(char operand)
{
  int a = this->_pop();

  //LOG(operand); LOGLN(a);
  
  switch (operand) {
    case '!':
      this->_push(!a);
      break;

    default:
      // preserve spec: consume only 1 item in the stack
      this->_push(a);
  }
}

void RpnSolver::_compute2(char operand)
{
  int b = this->_pop();
  int a = this->_pop();

  //LOG(a); LOG(operand); LOGLN(b);
  
  switch (operand) {
    case '+':
      this->_push(a + b);
      break;

    case '-':
      this->_push(a - b);
      break;

    case '*':
      this->_push(a * b);
      break;

    case '/':
      this->_push(a / b);
      break;

    case '<':
      this->_push(a < b);
      break;

    case '=':
      this->_push(a == b);
      break;

    case '>':
      this->_push(a > b);
      break;

    default:
      // preserve spec: consume only 1 item in the stack
      this->_push(a);
  }
}


void RpnSolver::_compute3(char operand)
{
  int test = this->_pop();
  int ifFalse = this->_pop();
  int ifTrue = this->_pop();

  //LOG(test); LOG(operand); LOG(ifTrue); LOG(':'); LOGLN(ifFalse);
  
  switch (operand) {
    case '?':
      if (test) {
        this->_push(ifTrue);
      } else {
        this->_push(ifFalse);
      }
      break;

    default:
      // preserve spec: consume only 1 item in the stack
      this->_push(ifTrue);
  }
}
