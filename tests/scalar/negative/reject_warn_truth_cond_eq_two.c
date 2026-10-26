// expect: rejected cerastes-truth_value_compare @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R6: `(a > b ? true : false) == 2` (-Wtautological-constant-out-of-range-compare / -Wbool-compare) (clang, and gcc C++)
#include <cerastes.h>
#include <stdbool.h>

int test(int a, int b, bool p, bool q)
{
  return (a > b ? true : false) == 2;
}
