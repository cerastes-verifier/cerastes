// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 4: unsigned arithmetic wraps; no overflow error
#include <cerastes.h>

unsigned dec(unsigned a)
    CERASTES_REQUIRES(a == 0u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return a - 1u;
}

unsigned inc_max(unsigned a)
    CERASTES_REQUIRES(a == 4294967295u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a + 1u;
}
