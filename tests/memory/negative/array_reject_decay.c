// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 10: the decay of an array outside a subscript is rejected until item 10's rows land (this pin is deleted by Part D) (lifted by Part D of M4.5e1a)
// Pre-registered from record 0059 item 10
#include <cerastes.h>

int first(void)
{
  int a[3] = {1, 2, 3};
  int *p = a;
  return *p;
}
