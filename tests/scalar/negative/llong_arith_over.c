// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/llong_arith.c
// Pins: 0010 item 3: 2^62 * 2 does not fit long long
#include <cerastes.h>

long long twice(long long a)
    CERASTES_REQUIRES(a >= 0 && a <= 4611686018427387904LL)
    CERASTES_ENSURES(r, r >= a)
{
  return a * 2;
}
