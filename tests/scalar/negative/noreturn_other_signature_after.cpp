// expect: rejected cerastes-annotation_reserved_library @ L8
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 6 and question 117 (was 0015 item 5: code after a call of exit(void) was reachable): the global exit is the library's name, so a user contract on it is rejected before any call is looked at
// The library exit(int) ends the path (tests/vlibc/exit_explicit.c); a user exit(void) with a contract is refused.
#include <cerastes.h>

void exit(void)
    CERASTES_ENSURES(true);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    exit();
    CERASTES_CHECK(x <= 5);
  }
  return x;
}
