// expect: rejected cerastes-complement_of_truth_value @ L8
// mode: c23, c++20
// twin: tests/scalar/static_assert_closed.c
// Pins: 0024 item 4 (amends 0023 item 5): ~ of a bool is -2, not false (-Wbool-operation)
#include <cerastes.h>
#include <stdbool.h>

static_assert(~true != 0, "rejected");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
