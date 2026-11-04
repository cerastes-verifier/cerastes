// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function with only a declaration and a contract: callers rely on it, nothing checks it here.
// tag::source[]
#include <cerastes.h>

int next_id(int last)
    CERASTES_REQUIRES(0 <= last && last < 1000)
    CERASTES_ENSURES(r, r == last + 1);

int two_ids(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = next_id(0);
  return next_id(a);
}
// end::source[]
