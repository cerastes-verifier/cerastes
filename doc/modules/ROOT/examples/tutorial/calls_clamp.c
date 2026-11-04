// expect: verified
// expect: native-ok
// mode: c23, c++20
// A caller is verified against the callee's contract alone, so the contract must be precise enough.
// tag::source[]
#include <cerastes.h>

int clamp(int x, int lo, int hi)
    CERASTES_REQUIRES(lo <= hi)
    CERASTES_ENSURES(r, lo <= r && r <= hi
                        && (!(lo <= x && x <= hi) || r == x)
                        && (!(x < lo) || r == lo)
                        && (!(x > hi) || r == hi))
{
  if (x < lo) {
    return lo;
  }
  if (x > hi) {
    return hi;
  }
  return x;
}

int use_clamp(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a = clamp(5, 0, 10);
  int b = clamp(-3, 0, 10);
  return a - b;
}
// end::source[]
