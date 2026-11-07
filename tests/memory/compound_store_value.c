// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2: *p op= e used as a value is the new value of the cell, the operation checked like *p = *p op e (the builder's compound store, record 0053 item 10)
#include <cerastes.h>

int shrink_through(int k, int d)
    CERASTES_REQUIRES(0 <= k && 1 <= d)
    CERASTES_ENSURES(r, r == k / d)
{
  int x = k;
  int *p = &x;
  int t = (*p /= d);
  CERASTES_CHECK(x == t);
  return t;
}
