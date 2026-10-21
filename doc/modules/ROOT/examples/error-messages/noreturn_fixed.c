// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: the function that does not return has no condition; its callers test it.
// tag::source[]
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(void)
{
  abort();
}

int check_positive(int x)
    CERASTES_ENSURES(r, r > 0)
{
  if (x <= 0) {
    fail();
  }
  return x;
}
// end::source[]
