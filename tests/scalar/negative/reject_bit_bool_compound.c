// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/scalar/bit_compound.c
// Pins: 0023 item 3: a compound assignment to a bool stays rejected (0012)
#include <cerastes.h>
#include <stdbool.h>

bool update(bool flag, int c)
{
  flag &= c;
  return flag;
}
