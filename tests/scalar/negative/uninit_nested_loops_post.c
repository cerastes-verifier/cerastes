// expect: error postcondition @ L28
// mode: c23, c++20
// twin: tests/scalar/uninit_nested_loops.c
// Pins: 0019 item 4: the same nest, assigned and read; the result is wrong
#include <cerastes.h>

int nested(int n)
    CERASTES_REQUIRES(1 <= n && n <= 20)
    CERASTES_ENSURES(r, r == n - 1)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i < n)
  CERASTES_DECREASES(n - i)
  do {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 1)
    CERASTES_DECREASES(1 - j)
    for (;;) {
      x = i + 1;
      if (j >= 1) {
        break;
      }
      j += 1;
    }
    i += 1;
  } while (i < n);
  return x;
}
