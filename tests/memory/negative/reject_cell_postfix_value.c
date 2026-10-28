// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 items 9, 10: x++ used as a value writes a cell inside an expression (re-expected by b1c)
#include <cerastes.h>

int post_inc_value(void)
{
  int x = 1;
  int *p = &x;
  int y = x++;
  return y + *p;
}
