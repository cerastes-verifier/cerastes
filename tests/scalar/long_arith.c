// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 items 2 and 3: long is wider than int on this target (from EDG, not constants)
// (long)a + b cannot overflow; the second function is exact at 2^62.
#include <cerastes.h>

long widen_add(int a, int b)
    CERASTES_ENSURES(r, r == (long)a + b)
{
  return (long)a + b;
}

long add_long(long a, long b)
    CERASTES_REQUIRES(a >= 0 && a <= 4611686018427387903L && b >= 0 && b <= 4611686018427387903L)
    CERASTES_ENSURES(r, r >= a)
{
  return a + b;
}
