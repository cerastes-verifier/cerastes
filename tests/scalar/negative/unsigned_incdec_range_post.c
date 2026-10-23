// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/unsigned_incdec_range.c
// Pins: 0022 item 1: u++ at 4294967294u gives 4294967295u, not 4294967294u
#include <cerastes.h>

unsigned inc(unsigned u)
    CERASTES_REQUIRES(u == 4294967294u)
    CERASTES_ENSURES(r, r == 4294967294u)
{
  u++;
  return u;
}
