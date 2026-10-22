// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/scalar/do_const_measured.c
// Pins: 0017 item 7: the loop is measured and its body verified (the exact ensures notices a wrong count)
// use() is changed to the sum the wrong ensures would give, so the error is only the one in once.
#include <cerastes.h>

const int Z = 0;

int once(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n + 2)
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
    CERASTES_ENSURES(r, r == 103)
{
  int a = once(0);
  int b = once(99);
  return a + b;
}
