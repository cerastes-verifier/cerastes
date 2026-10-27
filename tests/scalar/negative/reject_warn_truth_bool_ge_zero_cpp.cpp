// expect: rejected cerastes-truth_value_compare @ L9
// mode: c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R6: reject_warn_truth_bool_ge_zero.c in C++ (EDG prints no warning there)
#include <cerastes.h>

int test(bool p)
{
  return p >= 0;
}
