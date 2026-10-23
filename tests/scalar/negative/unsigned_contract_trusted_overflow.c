// expect: error overflow @ L8
// mode: c23, c++20
// twin: tests/scalar/unsigned_contract_wf.c
// Pins: 0022 item 2: a + b in the ensures of a bodyless declaration may overflow: an overflow at the ensures line, not UINT_MAX
#include <cerastes.h>

unsigned add(unsigned a, unsigned b)
    CERASTES_ENSURES(r, r == a + b);

unsigned use(unsigned x)
    CERASTES_REQUIRES(x < 10u)
{
  return add(x, 1u);
}
