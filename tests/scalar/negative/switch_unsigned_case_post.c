// expect: error postcondition @ L13
// c-only: C converts a case value to the promoted type of the scrutinee; in C++ the narrowing -1 is ill-formed
// twin: tests/scalar/switch_unsigned_case.c
// Pins: 0014 item 6: case -1 matches UINT_MAX (the postcondition would hold if it never matched)
// Claiming r == 0 always forgets that UINT_MAX matches the case.
#include <cerastes.h>

unsigned int is_all_ones(unsigned int u)
    CERASTES_ENSURES(r, r == 0)
{
  switch (u) {
  case -1:
    return 1;
  default:
    return 0;
  }
}
