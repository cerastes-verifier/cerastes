// expect: error overflow @ L9
// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_contract_wf.c
// Pins: 0022 item 2: a + b may overflow in the ensures (its well-definedness, at the clause) and in the body (at the return)
#include <cerastes.h>

unsigned add_body(unsigned a, unsigned b)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}
