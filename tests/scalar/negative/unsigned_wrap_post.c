// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/unsigned_wrap.c
// Pins: 0010 item 4: 0u - 1u is UINT_MAX, not 0
// The wrong expectation is a postcondition failure, not an overflow.
#include <cerastes.h>

unsigned dec(unsigned a)
    CERASTES_REQUIRES(a == 0u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a - 1u;
}

unsigned inc_max(unsigned a)
    CERASTES_REQUIRES(a == 4294967295u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a + 1u;
}
