// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23
// c-only: array compound literals are C only (a GNU extension in C++)
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1: an array compound literal is rejected (C only: in C++ it is a GNU extension) (lifted by M4.5e1e)
// Pre-registered from record 0059 item 1
#include <cerastes.h>

int f(void)
{
  return (int[]){1, 2, 3}[0];
}
