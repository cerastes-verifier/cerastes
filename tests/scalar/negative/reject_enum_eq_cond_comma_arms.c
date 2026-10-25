// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_eq_comma_enumerator.c
// Pins: 0023 item 7 (audit round 1): the arms of the ?: may be commas ending in enumerators of one enumeration: rejected in both modes as C++ rejects it
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(enum color c, int x, int y)
    CERASTES_REQUIRES(y < 2147483646)
{
  return c == (x ? (y++, RED) : (y++, GREEN));
}
