// expect: error overflow @ L13
// mode: c23, c++20
// twin: tests/memory/bump_guarded.c
// Pins: 0032 items 6, 9: the guard is gone and (*p)++ overflows at INT_MAX, reported at the operator
#include <cerastes.h>

int bump_guarded(int k)
    CERASTES_REQUIRES(0 <= k)
    CERASTES_ENSURES(r, r >= k)
{
  int x = k;
  int *p = &x;
  (*p)++;
  return x;
}
