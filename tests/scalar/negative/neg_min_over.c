// expect: error overflow @ L9
// mode: c23, c++20
// twin: tests/scalar/neg_min.c
// Pins: 0010 item 3: -INT_MIN is not an int
#include <cerastes.h>

int negate(int a)
{
  return -a;
}
