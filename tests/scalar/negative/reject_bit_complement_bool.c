// expect: rejected cerastes-complement_of_truth_value @ L10
// mode: c23, c++20
// twin: tests/scalar/bit_not.c
// Pins: 0023 item 5: ~ of a bool (both compilers warn: -Wbool-operation)
#include <cerastes.h>
#include <stdbool.h>

int flip(bool b)
{
  return ~b;
}
