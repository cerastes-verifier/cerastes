// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_contract_wf.c
// Pins: 0022 item 2: a + b is not a (b may be positive)
#include <cerastes.h>

unsigned add_body(unsigned a, unsigned b)
    CERASTES_REQUIRES(a <= 4294967295u - b)
    CERASTES_ENSURES(r, r == a)
{
  return a + b;
}
