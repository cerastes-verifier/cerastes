// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_incdec_range.c
// Pins: 0022 item 1: u++ at UINT_MAX is an overflow at the operator
#include <cerastes.h>

unsigned inc(unsigned u)
    CERASTES_REQUIRES(u == 4294967295u)
    CERASTES_ENSURES(r, r == 0u)
{
  u++;
  return u;
}
