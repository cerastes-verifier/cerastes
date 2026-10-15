// expect: frontend-error friend_attribute_requires_definition @ L10
// expect: native-ok
// twin: tests/annotations/contracts/member_inclass.cpp
// Mutation of the twin: a contract on a non-defining friend declaration (existing EDG diagnostic).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct counter {
  int val;
  friend int peek(const counter& c) CERASTES_REQUIRES(c.val >= 0);
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val)
  {
    return val;
  }
  void bump() CERASTES_REQUIRES(val < 100) CERASTES_ENSURES(val == cerastes::old(val) + 1)
  {
    val += 1;
  }
};
