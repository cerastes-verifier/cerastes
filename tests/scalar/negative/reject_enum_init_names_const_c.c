// expect: rejected const_var_in_C_const_expr @ L10
// c-only: C++ accepts a const int initialized with a constant in an enumerator initializer
// twin: tests/scalar/enum_init.c
// Pins: 0018 items 2 and 10, 0017 item 2: in C a const object is not an integer constant expression: error #1901 in verify mode
#include <cerastes.h>

const int K = 1;

enum {
  A = K
};

int f(int x)
{
  return x + A;
}
