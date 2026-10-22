// expect: rejected const_var_in_C_const_expr @ L11
// c-only: C++ accepts a const local in a case value (switch_case_named.cpp); C23 does not (6.6p8) and constexpr does not parse here
// twin: tests/scalar/switch_case_named.cpp
// Pins: 0017 item 2: in C a const object is not an integer constant expression: EDG refuses case k (error #1901 in verify mode)
#include <cerastes.h>

int pick(int x)
{
  const int k = 5;
  switch (x) {
  case k:
    return 1;
  default:
    return 0;
  }
}
