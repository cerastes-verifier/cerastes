// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0017 item 7: do ... while (Z) with Z a const global equal to 0 is a measured loop in both modes
// Z names a variable, so the loop is not exempt from the measure; the body runs once because Z is 0.
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
  CERASTES_DECREASES(Z)
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
