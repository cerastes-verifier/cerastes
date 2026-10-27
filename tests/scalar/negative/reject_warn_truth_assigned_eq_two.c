// expect: rejected cerastes-truth_value_compare @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R6: the value of an assignment of a truth value is that truth value (clang and gcc warn)
#include <cerastes.h>

int test(int a, int b, int y)
{
  int t = (y = (a > b)) == 2;
  return t + y;
}
