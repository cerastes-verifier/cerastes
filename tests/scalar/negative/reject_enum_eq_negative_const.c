// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_eq_signed_ok.c
// Pins: 0022 item 5 (owner, 2026-10-23): a named constant below zero: rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
const int M = -1;
int f(enum color c)
{
  return c == M;
}
