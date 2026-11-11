// expect: rejected cerastes-unsupported_declaration @ L12
// mode: c23
// c-only: typeof is a C23 keyword (C++20 has decltype: the _decltype twin)
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1 (as built): an array local whose type is written with typeof is rejected, at the local's declaration, by the rule "an array type written as an array declarator" (lifted by M4.5e1e)
// Pre-registered from the edg-backend report 3 of M4.5e1a
#include <cerastes.h>

int f(void)
{
  int a[3] = {1, 2, 3};
  typeof(a) b = {1, 2, 3};
  return a[0] + b[0];
}
