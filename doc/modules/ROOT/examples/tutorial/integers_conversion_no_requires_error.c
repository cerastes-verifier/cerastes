// expect: error conversion @ L12
// mode: c23, c++20
// The range precondition is gone, so the conversion to short may change the value.
// Derived from integers_conversion.c.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

short to_short(int x)
    CERASTES_ENSURES(r, r == x)
{
  return (short)x;
}
// end::source[]
