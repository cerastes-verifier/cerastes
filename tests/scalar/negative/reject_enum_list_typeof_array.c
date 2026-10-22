// expect: rejected cerastes-unsupported_expression @ L9
// c-only: C++ types the enumerator the same way inside the list and is right there (M is 16)
// twin: tests/scalar/enum_sizeof.c
// Pins: 0018 item 10: a typeof nested in an array type operand of sizeof sees A's C++-style type (C: int, M is 8): rejected in C
#include <cerastes.h>

enum {
  A = 5L,
  M = sizeof(typeof(A)[2])
};

int m_value(void)
{
  return M;
}
