// expect: rejected cerastes-unsupported_declaration @ L11
// mode: c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1 (as built): decltype(a) of an array local is a computed type (lifted by M4.5e1e)
// Pre-registered from the edg-backend report 3 of M4.5e1a
#include <cerastes.h>

int f(void)
{
  int a[3] = {1, 2, 3};
  decltype(a) b = {1, 2, 3};
  return a[0] + b[0];
}
