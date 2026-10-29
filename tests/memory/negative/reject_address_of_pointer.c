// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: the address of a pointer local is rejected (re-expected by M4.5b1c3)
#include <cerastes.h>

int address_of_pointer(void)
{
  int x = 0;
  int *p = &x;
  return &p == 0;
}
