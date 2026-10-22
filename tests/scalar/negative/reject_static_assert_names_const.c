// expect: rejected const_var_in_C_const_expr @ L8
// c-only: C++ accepts it (a const object is usable in a constant expression there); C23 does not (6.6p8) and constexpr does not parse here
// twin: tests/scalar/const_global_init_named.cpp
// Pins: 0017 item 2: in C a _Static_assert naming a const object is not a constant expression: EDG refuses it
#include <cerastes.h>

const int N = 8;
_Static_assert(N == 8, "N is 8");

int get(void)
{
  return N;
}
