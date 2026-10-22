// expect: error uninitialized @ L21
// mode: c23, c++20
// twin: tests/scalar/uninit_for_ever_break.c
// Pins: 0019 item 4: the break comes before the assignment, so the read after the loop may be indeterminate
#include <cerastes.h>

int find_ten(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (;;) {
    if (i >= n) {
      break;
    }
    x = i + 10;
    i += 1;
  }
  return x;
}
