// expect: rejected cerastes-unsupported_declaration @ L9
// expect: rejected cerastes-unsupported_declaration @ L13
// mode: c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1 (as built) and 0024 item 6: a using-alias of an array type is rejected at the alias, and an array local declared through it again at the local (lifted by M4.5e1e)
// Pre-registered from the edg-backend report 3 of M4.5e1a
#include <cerastes.h>

using T3 = int[3];

int f(void)
{
  T3 b = {1, 2, 3};
  return b[0];
}
