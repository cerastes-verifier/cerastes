// expect: rejected cerastes-missing_decreases @ L17
// mode: c23, c++20
// twin: tests/scalar/do_const_measured.c
// Pins: 0017 item 7: do ... while (Z) with Z a const global is a measured loop in both modes, so it needs a decreases
#include <cerastes.h>

const int Z = 0;

int once(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n + 1)
{
  CERASTES_CHECK(0 <= n);
  CERASTES_CHECK(n < 100);
  int i = n;
  CERASTES_INVARIANT(i == n)
  do {
    i += 1;
  } while (Z);
  return i;
}

int use(void)
    CERASTES_ENSURES(r, r == 101)
{
  int a = once(0);
  int b = once(99);
  return a + b;
}
