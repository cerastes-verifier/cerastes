// expect: rejected cerastes-unsequenced_modification @ L13
// mode: c23, c++20
// twin: tests/memory/cell_postfix_value.c
// Pins: 0039 item 3 (audit S9, owner): (p = q, (*p)++ + *p) modifies and reads *p with no sequence point between; GCC -Wsequence-point warns, so it is rejected
#include <cerastes.h>

int comma_pointer(void)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int *q = &y;
  return (p = q, (*p)++ + *p);
}
