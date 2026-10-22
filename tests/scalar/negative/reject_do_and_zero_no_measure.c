// expect: rejected cerastes-missing_decreases @ L11
// mode: c23, c++20
// twin: tests/scalar/do_count.c
// Pins: 0017 item 7: do ... while (0 && x) names a variable, so it is a measured loop and needs a decreases
#include <cerastes.h>

int test(int x)
    CERASTES_REQUIRES(0 <= x && x < 10)
{
  int i = x;
  do {
    i += 1;
  } while (0 && x);
  return i;
}
