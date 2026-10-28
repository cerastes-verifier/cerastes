// expect: rejected bad_initializer_type @ L11
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 3: C only warns (#144, initializing from an incompatible pointer; C++ refuses it), verify mode makes it an error
#include <cerastes.h>

int init_pointer(void)
{
  int a = 0;
  int *pi = &a;
  long *pl = pi;
  return pl == 0;
}
