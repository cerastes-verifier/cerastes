// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 3: a conversion of an integer other than a null pointer constant to a pointer is rejected (not lifted before M8)
#include <cerastes.h>

int int_to_pointer(void)
{
  int *p = (int *)4;
  return p == 0;
}
