// expect: rejected cerastes-call_without_contract @ L14
// c-only: C++ gives quick_exit C linkage in <stdlib.h>, so the user declaration is EDG error 337 there; platform: linux
// twin: tests/scalar/noreturn_libc.c
// Pins: 0015 item 5, 0031 item 10: the glibc twin of reject_quick_exit_user_declared.c: quick_exit is not covered by vlibc, wherever it is declared first
// A user declaration before <stdlib.h> gives quick_exit no contract; glibc's `__attribute__((__noreturn__))` on it is not trusted.
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
