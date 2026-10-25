// expect: rejected cerastes-complement_of_truth_value @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_not.c
// Pins: 0023 item 5: ~ of a comparison
#include <cerastes.h>

int flip(int x)
{
  return ~(x > 0);
}
