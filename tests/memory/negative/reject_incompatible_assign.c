// expect: rejected incompatible_assignment_operands @ L13
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 3: C only warns (#513, assigning an incompatible pointer; C++ refuses it), verify mode makes it an error
#include <cerastes.h>

int assign_pointer(void)
{
  int a = 0;
  long b = 0;
  int *pi = &a;
  long *pl = &b;
  pl = pi;
  return pl == 0;
}
