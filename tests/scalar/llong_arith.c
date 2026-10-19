// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 items 2 and 3: long long doubling at its exact bound
#include <cerastes.h>

long long twice(long long a)
    CERASTES_REQUIRES(a >= -4611686018427387904LL && a <= 4611686018427387903LL)
    CERASTES_ENSURES(r, r == a + a)
{
  return a * 2;
}
