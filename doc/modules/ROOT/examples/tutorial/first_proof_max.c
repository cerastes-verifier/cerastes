// expect: verified
// expect: native-ok
// mode: c23, c++20
// The result is one of the arguments and at least as large as both.
// tag::source[]
#include <cerastes.h>

int max_of(int a, int b)
    CERASTES_ENSURES(r, r >= a && r >= b && (r == a || r == b))
{
  if (a > b) {
    return a;
  }
  return b;
}
// end::source[]
