// expect: rejected cerastes-unsupported_expression @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_ok.c
// Pins: 0022 item 5 (owner, 2026-10-23): f == n: an enumeration with an unsigned fixed base against a value of another enumeration is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum neg { NA = -1, NB };
enum fu : unsigned { FA, FB };

int g(enum fu f, enum neg n)
{
  return f == n;
}
