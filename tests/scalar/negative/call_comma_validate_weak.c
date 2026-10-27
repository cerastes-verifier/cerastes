// expect: error precondition @ L21
// mode: c23, c++20
// twin: tests/scalar/call_comma_validate.c
// Pins: 0026 item 1: validate only promises y >= 0 now, so h(0) is possible and its precondition fails at the call
#include <cerastes.h>

// Returns only when y >= 0 (a trusted contract: the callee ends the program otherwise).
int validate(int y)
    CERASTES_ENSURES(r, y >= 0);

int h(int a)
    CERASTES_REQUIRES(a > 0)
    CERASTES_ENSURES(r, r == 100 / a)
{
  return 100 / a;
}

int checked(int y)
    CERASTES_ENSURES(r, 0 <= r && r <= 100)
{
  return h((validate(y), y));
}
