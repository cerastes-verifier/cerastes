// expect: verified
// expect: native-ok
// mode: c23, c++20
// A check states an intermediate fact the prover must establish.
// tag::source[]
#include <cerastes.h>

int clamp_to_100(int x)
    CERASTES_ENSURES(r, r <= 100)
{
  if (x > 100) {
    x = 100;
  }
  CERASTES_CHECK(x <= 100);
  return x;
}
// end::source[]
