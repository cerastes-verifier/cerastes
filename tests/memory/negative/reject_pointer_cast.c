// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 3: a cast between pointer types is rejected, whatever the pointees (not lifted before M8)
#include <cerastes.h>

int cast_pointer(void)
{
  int x = 0;
  long *pl = (long *)&x;
  return pl == 0;
}
