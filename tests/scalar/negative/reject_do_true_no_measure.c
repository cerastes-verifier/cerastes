// expect: rejected cerastes-missing_decreases @ L11
// mode: c23, c++20
// twin: tests/scalar/do_count.c
// Pins: 0013 item 3 and 0014 item 5: do ... while (1) needs a decreases
#include <cerastes.h>

int test(int n)
    CERASTES_REQUIRES(0 <= n && n < 10)
{
  int i = n;
  do {
    i += 1;
    if (i > 15) {
      break;
    }
  } while (1);
  return i;
}
