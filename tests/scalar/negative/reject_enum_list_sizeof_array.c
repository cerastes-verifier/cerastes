// expect: rejected cerastes-unsupported_expression @ L9
// c-only: C++ types the enumerator the same way inside the list and is right there (M is 2)
// twin: tests/scalar/enum_sizeof.c
// Pins: 0018 item 10: an array bound inside a sizeof type operand sees N's C++-style type (C: N is 4, M is 1): rejected in C
#include <cerastes.h>

enum {
  N = sizeof(int),
  M = sizeof(char[(N - 5) > 0 ? 2 : 1])
};

int m_value(void)
{
  return M;
}
