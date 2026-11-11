// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/array_loop_break.c
// Pins: 0059 item 8: the invariant allows i == 3 after the loop, so r < 3 is not known
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int find(void)
    CERASTES_ENSURES(r, r < 3)
{
  int a[3] = {5, 1, 2};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a[i] == 1) {
      break;
    }
    i = i + 1;
  }
  return i;
}
