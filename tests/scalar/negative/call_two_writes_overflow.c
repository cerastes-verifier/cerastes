// expect: error overflow @ L16
// mode: c23, c++20
// twin: tests/scalar/call_two_writes.c
// Pins: 0025 item 2: the sum of two stored results is still checked: it may overflow when the callees do not cap their results
#include <cerastes.h>

int plain(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == x);

int two_writes(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  int x;
  int y;
  int s = (x = plain(a)) + (y = plain(b));
  if (x < y) {
    return s;
  }
  return s;
}
