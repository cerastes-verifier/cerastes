// expect: verified
// expect: native-ok
// mode: c23, c++20
// Averaging without overflow: b - a cannot overflow when 0 <= a <= b.
// tag::source[]
#include <cerastes.h>

int average(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= b)
    CERASTES_ENSURES(r, a <= r && r <= b)
{
  return a + (b - a) / 2;
}
// end::source[]
