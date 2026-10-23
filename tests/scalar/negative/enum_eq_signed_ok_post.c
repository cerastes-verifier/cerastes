// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_eq_signed_ok.c
// Pins: 0022 item 5 (owner, 2026-10-23): c == 1 is not (int)c == 2
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int eq_const(enum color c)
    CERASTES_ENSURES(r, r == ((int)c == 2))
{
  return c == 1;
}
