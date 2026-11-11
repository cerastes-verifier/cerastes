// expect: error lifetime @ L20
// mode: c23, c++20
// twin: tests/memory/array_exit_switch_break.c
// Pins: 0059 item 6: a callee kept a[1]; the break out of the block reports lifetime (the second break is unreachable: a[0] is 1)
// Pre-registered from the M4.5e1a sweep (report 5, s12 keep)
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int pick(int c)
    CERASTES_ENSURES(r, r == (c == 1 ? 0 : 2))
{
  int v = 0;
  switch (c) {
  case 1: {
    int a[2] = {1, 2};
    dispose(&a[1]);
    if (a[0] == 1) {
      break;
    }
    v = a[1];
    break;
  }
  default:
    v = 2;
    break;
  }
  return v;
}
