// expect: frontend-error attr_result_name_void_function @ L14
// expect: native-ok
// twin: tests/annotations/contracts/member_inclass.cpp
// Mutation of the twin: a result name on a void member function.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct counter {
  int val;
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val)
  {
    return val;
  }
  void bump() CERASTES_REQUIRES(val < 100) CERASTES_ENSURES(r, val == cerastes::old(val) + 1)
  {
    val += 1;
  }
};
