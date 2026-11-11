// expect: rejected address_of_register_variable @ L13
// mode: c23
// c-only: in C++ a register declaration is EDG's error #2798 (the _cxx pin)
// twin: tests/memory/array_sum_two.c
// Pins: 0059 audit round 2 (B1): a register array decays to a pointer when subscripted, which C forbids (C23 6.3.2.1p3); EDG's warning #138 is an error in C verify mode
// Pre-registered from audit round 2 of record 0059: the error is reported at the subscript
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 2)
{
  register int a[2] = {1, 2};
  return a[1];
}
