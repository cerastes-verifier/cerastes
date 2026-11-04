// expect: error postcondition @ L20
// c-only: in C++ the global-namespace name exit is reserved whatever its linkage (decision 0046 item 6), so a static exit is refused there
// twin: tests/vlibc/static_exit.c
// Pins: 0046 item 6 (audit round 2, note 2): the static exit returns, so the path of x > 5 reaches the return; the library's exit would end it
// The postcondition claims x <= 5, which only the library's exit would justify.
#include <cerastes.h>

static void exit(int status)
{
  (void) status;
}

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 5)
{
  if (x > 5) {
    exit(1);
  }
  return x;
}
