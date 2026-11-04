// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: in C++ the global-namespace name exit is reserved whatever its linkage and a static one is refused there on its own (decision 0046 item 6)
// twin: tests/vlibc/static_exit.c
// Pins: 0046 item 6 (re-audit): a static DECLARATION of exit before <stdlib.h>, defined after it, is refused the same way
// Where the report lands (the program's declaration or vlibc's line) is the encoder's choice; the harness names the program's.
#include <cerastes.h>

static void exit(int status);
#include <stdlib.h>

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
