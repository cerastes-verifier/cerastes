// expect: rejected cerastes-unsequenced_modification @ L13
// mode: c23, c++20
// twin: tests/memory/cell_conditional_target.c
// Pins: 0039 item 3 (audit S1, static): *(*q > 0 ? p : q) = (*q)++ reads *q in the target's address and modifies it in the value, through one pointer variable: rejected like the compilers
#include <cerastes.h>

int conditional_target_same(void)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int *q = &y;
  *(*q > 0 ? p : q) = (*q)++;
  return x + y;
}
