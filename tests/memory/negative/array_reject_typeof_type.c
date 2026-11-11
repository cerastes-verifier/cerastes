// expect: rejected cerastes-unsupported_declaration @ L11
// mode: c23
// c-only: typeof is a C23 keyword
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1 (as built): typeof of an array type name is a computed type too (lifted by M4.5e1e)
// Pre-registered from the edg-backend report 3 of M4.5e1a
#include <cerastes.h>

int f(void)
{
  typeof(int[3]) b = {1, 2, 3};
  return b[0];
}
