// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/loop_conditional_owns_by_name.c
// Pins: 0038 item 3 (re-audit): the cell holds 10 after the loop, not 11
#include <cerastes.h>

int by_name(int *p, int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)) && r == 11)
{
  int x = 0;
  int *q = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && x == i)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    x = x + 1;
    i += 1;
  }
  return *q;
}
