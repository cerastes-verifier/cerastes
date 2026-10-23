// expect: error uninitialized @ L24
// mode: c23, c++20
// twin: tests/scalar/uninit_nested_loops.c
// Pins: 0019 item 4: the inner loop may run zero times, so x may not be assigned after the outer loop
#include <cerastes.h>

int nested(int n)
    CERASTES_REQUIRES(1 <= n && n <= 20)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i < n)
  CERASTES_DECREASES(n - i)
  do {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n)
    CERASTES_DECREASES(n - j)
    while (j < n) {
      x = i;
      j += 1;
    }
    i += 1;
  } while (i < n);
  return x;
}
