// expect: rejected cerastes-xor_as_power @ L7
// mode: c23, c++20
// twin: tests/scalar/static_assert_closed.c
// Pins: 0024 item 4: the xor-as-power detector runs on encoded assertions (the plan's unparenthesised `2 ^ 8 == 10` is another shape: -Wparentheses, a bitwise operand that is a comparison)
#include <cerastes.h>

static_assert((2 ^ 8) == 10, "rejected");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
