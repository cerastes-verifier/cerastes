// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function declared not to return ends the path of its caller; abort never returns.
// tag::source[]
#include <limits.h>
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(void)
{
  abort();
}

// a / b; the program ends where the division would be undefined.
int checked_div(int a, int b)
{
  if (b == 0 || (a == INT_MIN && b == -1)) {
    fail();
  }
  return a / b;
}
// end::source[]
