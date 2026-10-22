// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: r is assigned on every path that reaches the return.
// tag::source[]
#include <cerastes.h>

int positive_or_none(int a)
    CERASTES_ENSURES(res, res == (a > 0 ? a : -1))
{
  int r;
  if (a > 0) {
    r = a;
  } else {
    r = -1;
  }
  return r;
}
// end::source[]
