// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_ok.c
// Pins: 0022 item 5 (owner, 2026-10-23): c == (int)b: a bool cast explicitly to int is not a truth value (clang warns), so it is rejected like a signed int
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(enum color c, bool b)
{
  return c == (int)b;
}
