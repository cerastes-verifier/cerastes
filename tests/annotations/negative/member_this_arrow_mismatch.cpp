// expect: frontend-error attr_redeclaration_mismatch @ L19
// expect: native-ok
// twin: tests/annotations/contracts/member_outofclass.cpp
// Mutation of the twin: documented limitation: `this->val` and `val` mean the same but are not the same expression, so repeating the contract spelled differently is a mismatch.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

struct counter {
  int val;
  void set(int v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(val == v);
  int get() const CERASTES_REQUIRES(val >= 0) CERASTES_ENSURES(r, r == val);
};

void counter::set(int w) CERASTES_REQUIRES(w >= 0) CERASTES_ENSURES(val == w)
{
  val = w;
}

int counter::get() const CERASTES_REQUIRES(this->val >= 0)
{
  return val;
}
