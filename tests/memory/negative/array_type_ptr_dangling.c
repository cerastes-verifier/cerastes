// expect: error lifetime @ L15
// mode: c23, c++20
// twin: tests/memory/array_type_ptr.c
// Pins: 0059 item 2 (0032 item 6): a pointer element that outlives the block of the object it points to is used after that object ended (the return)
// Pre-registered from the M4.5e1a sweep (report 5, x07)
#include <cerastes.h>

int *get(void)
{
  int *ps[1];
  {
    int x = 0;
    ps[0] = &x;
  }
  return ps[0];
}
