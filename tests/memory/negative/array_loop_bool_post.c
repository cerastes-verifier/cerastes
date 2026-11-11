// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/array_loop_bool.c
// Pins: 0059 item 8: the invariant allows c == 3 after the loop, so r < 3 is not known (exact counting needs e1c's quantified invariants)
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int count_true(void)
    CERASTES_ENSURES(r, r < 3)
{
  bool a[3] = {true, false, true};
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= c && c <= i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a[i]) {
      c = c + 1;
    }
    i = i + 1;
  }
  return c;
}
