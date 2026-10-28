// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 3: a conversion of a pointer to an integer is rejected (not lifted before M8)
#include <cerastes.h>

int pointer_to_int(void)
{
  int x = 0;
  int *p = &x;
  long n = (long)p;
  return n == 0;
}
