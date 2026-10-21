// expect: rejected cerastes-call_without_contract @ L14
// c-only: C++ gives quick_exit C linkage in <stdlib.h>, so the user declaration is EDG error 337 there
// twin: tests/scalar/noreturn_libc.c
// Pins: 0015 item 5: the table is a closed list of names; quick_exit is not in it, wherever it is declared first
// A user declaration before <stdlib.h> gives quick_exit no contract; the call must still be rejected, not trusted.
void quick_exit(int);
#include <stdlib.h>
#include <cerastes.h>

int leave(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (x > 5) {
    quick_exit(1);
  }
  return x;
}
