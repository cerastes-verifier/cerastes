// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_eq_signed_ok.c
// Pins: 0022 item 5 (owner, 2026-10-23): plain char is signed here: rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(enum color c, char v)
{
  return c == v;
}
