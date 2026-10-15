// expect: frontend-error wrong_entity_for_attribute @ L8
// expect: native-ok
// twin: tests/annotations/contracts/member_inclass.cpp
// Mutation of the twin: requires on a class.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct CERASTES_REQUIRES(true) counter {
  int val;
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val)
  {
    return val;
  }
  void bump() CERASTES_REQUIRES(val < 100) CERASTES_ENSURES(val == cerastes::old(val) + 1)
  {
    val += 1;
  }
};
