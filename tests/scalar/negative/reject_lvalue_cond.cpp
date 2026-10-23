// expect: rejected cerastes-unsupported_expression @ L9
// mode: c++20
// twin: tests/scalar/assign_value.c
// Pins: 0020: (c ? x : y) = 1 uses an expression as an lvalue (C++ only; lvalue_var refuses it)
#include <cerastes.h>

int f(int a, int c, int x, int y)
{
  (c ? x : y) = 1;
  return x + y + a + c;
}
