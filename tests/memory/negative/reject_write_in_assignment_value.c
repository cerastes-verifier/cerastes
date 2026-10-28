// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 items 9, 10: *p = (*p)++ writes the cell inside the value of a write (re-expected by b1c)
#include <cerastes.h>

int self_assign(void)
{
  int x = 1;
  int *p = &x;
  *p = (*p)++;
  return x;
}
