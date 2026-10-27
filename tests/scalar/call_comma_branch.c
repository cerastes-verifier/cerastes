// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 1 and 2: a comma inside the right operand of && (a branch with a call decides its own step): h((validate(y), y)) is reached only when a != 0 and then validate has established y > 0; validate requires y <= 1000u and test only promises it when a != 0, so the operands of && cannot be swapped
#include <cerastes.h>

// Returns only when y > 0 (a trusted contract: the callee ends the program otherwise).
unsigned validate(unsigned y)
    CERASTES_REQUIRES(y <= 1000u) CERASTES_ENSURES(r, y > 0u);

unsigned h(unsigned a)
    CERASTES_REQUIRES(a > 0u)
    CERASTES_ENSURES(r, r == a - 1u)
{
  return a - 1u;
}

int test(int a, unsigned y)
    CERASTES_REQUIRES(a == 0 || y <= 1000u) CERASTES_ENSURES(r, r == (a != 0 && y > 1u))
{
  return a && h((validate(y), y)) > 0u;
}
