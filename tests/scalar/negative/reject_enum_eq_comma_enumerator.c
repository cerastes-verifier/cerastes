// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_eq_comma_enumerator.c
// Pins: 0023 item 7 (audit round 1): (x++, GREEN) is a value of the enumeration in both modes, so == with a signed int that may be negative is rejected as 0022 item 5 says
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(int x, int y)
    CERASTES_REQUIRES(x < 2147483647)
{
  return (x++, GREEN) == y;
}
