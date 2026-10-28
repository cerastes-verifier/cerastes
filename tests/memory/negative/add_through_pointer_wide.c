// expect: error overflow @ L13
// mode: c23, c++20
// twin: tests/memory/add_through_pointer.c
// Pins: 0032 items 6, 9: the bound is one too wide, so *p += k overflows at k == 1073741824
#include <cerastes.h>

int add_through(int k)
    CERASTES_REQUIRES(0 <= k && k <= 1073741824)
    CERASTES_ENSURES(r, r >= k)
{
  int x = k;
  int *p = &x;
  *p += k;
  return x;
}
