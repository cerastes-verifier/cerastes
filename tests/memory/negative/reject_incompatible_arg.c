// expect: rejected incompatible_param @ L13
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 3: C only warns (#167, an incompatible pointer argument; C++ refuses it), verify mode makes it an error
#include <cerastes.h>

void take_long(long *q);

int call_incompatible(void)
{
  int a = 0;
  int *pi = &a;
  take_long(pi);
  return a;
}
