// expect: verified
// expect: native-ok
// mode: c23, c++20
// One C source that means the same in C23 and in C++20.
// tag::source[]
#include <cerastes.h>

int clamp(int x, int lo, int hi)
    CERASTES_REQUIRES(lo <= hi)
    CERASTES_ENSURES(r, lo <= r && r <= hi)
{
  if (x < lo) {
    return lo;
  }
  if (x > hi) {
    return hi;
  }
  return x;
}
// end::source[]
