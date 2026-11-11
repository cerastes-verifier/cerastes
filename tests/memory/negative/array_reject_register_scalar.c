// expect: rejected address_of_register_variable @ L13
// mode: c23
// c-only: in C++ a register declaration is EDG's error #2798 (the _cxx pin)
// twin: tests/memory/array_sum_two.c
// Pins: 0059 audit round 2 (B1): the address of a register scalar (C23 6.5.3.2p1) has been accepted since M4.5b1a; EDG's warning #138 is an error in C verify mode
// Pre-registered from audit round 2 of record 0059
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 2)
{
  register int x = 1;
  int *p = &x;
  *p = 2;
  return x;
}
