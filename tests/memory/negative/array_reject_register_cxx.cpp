// expect: rejected register_keyword_disallowed @ L11
// mode: c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 audit round 2 (B1): register is not a storage class in C++17 and later: EDG's error #2798, at the declaration
// Pre-registered from audit round 2 of record 0059
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 2)
{
  register int a[2] = {1, 2};
  return a[1];
}
