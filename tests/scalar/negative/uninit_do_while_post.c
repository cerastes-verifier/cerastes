// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/scalar/uninit_do_while.c
// Pins: 0019 item 4: the condition reads the assigned x; the result is not zero
#include <cerastes.h>

int count_down(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i < n)
  CERASTES_DECREASES(n - i)
  do {
    x = n - i;
    i += 1;
  } while (x > 1);
  return x;
}
