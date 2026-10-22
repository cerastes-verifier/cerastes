// expect: rejected cerastes-unsupported_expression @ L9
// c-only: C++ types the enumerator the same way inside the list and is right there; typeof is C23
// twin: tests/scalar/enum_list_type.cpp
// Pins: 0018 item 10: inside the list C++ types A as unsigned int and C23 as int, so (typeof(A))-1 > 0 differs: rejected in C
#include <cerastes.h>

enum {
  A = 5u,
  B = (typeof(A))-1 > 0
};

int b_value(void)
{
  return B;
}
