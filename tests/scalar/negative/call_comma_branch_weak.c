// expect: error precondition @ L21
// mode: c23, c++20
// twin: tests/scalar/call_comma_branch.c
// Pins: 0026 items 1 and 2: validate only promises y >= 0, so h(0) is reachable inside the branch
#include <cerastes.h>

// Returns only when y >= 0 (a trusted contract: the callee ends the program otherwise).
unsigned validate(unsigned y)
    CERASTES_REQUIRES(y <= 1000u) CERASTES_ENSURES(r, y >= 0u);

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
