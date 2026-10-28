// expect: rejected cerastes-unsupported_expression @ L9
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: the address of a parameter is rejected (re-expected by b1b)
#include <cerastes.h>

int address_of_param(int a)
{
  int *p = &a;
  return *p;
}
