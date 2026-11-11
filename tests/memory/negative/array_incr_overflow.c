// expect: error overflow @ L16
// mode: c23, c++20
// twin: tests/memory/array_compound_ops.c
// Pins: 0059 item 4 and 0022: a[0]++ at INT_MAX overflows at the operator
// Pre-registered from record 0059 item 4
#include <cerastes.h>

#include <limits.h>

int bump(void)
    CERASTES_ENSURES(r, r == INT_MAX)
{
  int a[2] = {INT_MAX - 2, 0};
  a[0] += 1;
  a[0]++;
  a[0]++;
  a[1]--;
  return a[0];
}
