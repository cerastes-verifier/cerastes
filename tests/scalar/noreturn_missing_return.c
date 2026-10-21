// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 4: a non-void function that ends in a call of a [[noreturn]] function has no missing-return
// The last statement is the call, so the closing brace is unreachable.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(void)
{
  abort();
}

int must_be_positive(int x)
    CERASTES_ENSURES(r, r == x && x > 0)
{
  if (x > 0) {
    return x;
  }
  fail();
}
