// expect: error shift @ L7
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 4: a function with no contract has no __wf method; the constant is still checked in the body's method
#include <cerastes.h>

const int A = 1 << 40;

int get(void)
{
  return A;
}
