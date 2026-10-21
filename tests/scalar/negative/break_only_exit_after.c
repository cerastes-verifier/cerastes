// expect: error check @ L21
// mode: c23, c++20
// twin: tests/scalar/break_only_exit.c
// Pins: 0014 item 4: the check holds on the break exit only; the condition exit is possible
// With the weaker invariant the loop may end by i == 100.
#include <cerastes.h>

int stop_at(int limit)
    CERASTES_REQUIRES(0 <= limit && limit <= 50)
    CERASTES_ENSURES(r, r == limit)
{
  int i = 0;
  CERASTES_INVARIANT(i <= 100)
  CERASTES_DECREASES(100 - i)
  while (i < 100) {
    if (i == limit) {
      break;
    }
    i += 1;
  }
  CERASTES_CHECK(i == limit);
  return i;
}
