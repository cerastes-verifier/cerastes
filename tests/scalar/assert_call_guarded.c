// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 and 0015 item 6: an assert is checked like a check; a call in its guarded operand runs only when the guard allows
#include <assert.h>
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int test(int a)
    CERASTES_ENSURES(r, r == a)
{
  assert(a <= 0 || pos(a) > 0);
  return a;
}
