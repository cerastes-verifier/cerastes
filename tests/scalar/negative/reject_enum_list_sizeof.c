// expect: rejected cerastes-unsupported_expression @ L9
// c-only: C++ types the enumerator the same way inside the list and is right there (B is 8)
// twin: tests/scalar/enum_sizeof.c
// Pins: 0018 item 10: inside the list C++ types A as long (sizeof 8) and C23 as int (sizeof 4), so sizeof(A) differs: rejected in C
#include <cerastes.h>

enum {
  A = 5L,
  B = sizeof(A)
};

int b_value(void)
{
  return B;
}
