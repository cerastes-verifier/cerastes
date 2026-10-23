// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 (from reject_effect_call_guarded.c): an assignment of a call's result in the guarded operand of && runs only when the guard holds
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x && r > 0)
{
  return x;
}

int f(int a)
    CERASTES_ENSURES(r, r == (a > 5 ? a : 0))
{
  int x;
  if (a > 0 && (x = pos(a)) > 5) {
    return x;
  }
  return 0;
}
