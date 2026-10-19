// expect: rejected cerastes-contract_on_main @ L7
// mode: c23, c++20
// twin: tests/scalar/main_implicit.c
// Pins: 0011 item 7: a requires on main is rejected (nothing calls main, the precondition would never be checked)
#include <cerastes.h>

int main(void) CERASTES_REQUIRES(1 == 1)
{
  return 0;
}
