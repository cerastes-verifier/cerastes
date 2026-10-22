// expect: rejected cerastes-missing_decreases @ L13
// mode: c23, c++20
// twin: tests/scalar/do_const_measured.c
// Pins: 0017 item 7: do ... while (k) with k a local const is a measured loop in both modes, so it needs a decreases
#include <cerastes.h>

int once(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n + 1)
{
  const int k = 0;
  int i = n;
  do {
    i += 1;
  } while (k);
  return i;
}
