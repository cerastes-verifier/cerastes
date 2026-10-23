// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_ok.c
// Pins: 0022 item 5 (owner, 2026-10-23): RED == NB: enumerators of two different enumerations are rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum neg { NA = -1, NB };

int f(void)
{
  return RED == NB;
}
