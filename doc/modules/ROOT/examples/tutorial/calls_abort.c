// expect: verified
// expect: native-ok
// mode: c23, c++20
// abort() never returns, so code after the test knows the input was good.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int checked_half(int n)
    CERASTES_ENSURES(r, r == n / 2 && n % 2 == 0)
{
  if (n % 2 != 0) {
    abort();
  }
  return n / 2;
}
// end::source[]
