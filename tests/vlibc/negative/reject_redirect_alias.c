// expect: rejected cerastes-unsupported_attribute @ L10
// c-only: an alias needs its target's own symbol name, which a C++ definition mangles; the `.cpp` twin writes extern "C"
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 7 (question 118; item 8's alias rejection is reachable only from the platform's own text): a user-written GNU alias on a routine the SDK declares is an attribute outside the allow-list, so exit's `ensures(false)` is never trusted past it
// Before the allow-list reached it this program verified: exit(x) was trusted not to return, and f returned 0 against ensures(r == 1).
#include <stdlib.h>
#include <cerastes.h>

void my_exit(int status) { (void) status; }
void exit(int status) __attribute__((alias("my_exit")));

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
