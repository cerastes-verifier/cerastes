// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 5, 6, 8, 10: owns(*p) and old(*p) across a call, the callee's ensures come back, the caller's scope exit still sees the whole cell
#include <cerastes.h>
#include <stddef.h>

int inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int use_inc(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 5;
  int v = inc(&x);
  CERASTES_CHECK(x == 6);
  return v;
}
