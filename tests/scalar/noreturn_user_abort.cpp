// expect: rejected cerastes-annotation_reserved_library @ L7
// expect: native-ok
// Pins: 0046 item 6 and question 117 (was 0015 item 5: a user abort() in C++ was an ordinary function; C++ only, as C compilers know abort as a builtin): a global abort is the library's name, so a contract and a definition of it are rejected
// abort's contract comes from vlibc's <stdlib.h> only (a user's own `ensures(true)` here is refused, with or without the header).
#include <cerastes.h>

void abort(void) CERASTES_ENSURES(true)
{
  return;
}

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    abort();
  }
  return x;
}
