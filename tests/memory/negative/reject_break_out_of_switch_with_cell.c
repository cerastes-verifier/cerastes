// expect: rejected cerastes-unsupported_statement @ L17
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0032 item 10: a break that leaves a switch whose body declares a cell is rejected (re-expected by b1c)
#include <cerastes.h>

int switch_cell(int code)
{
  int total = 0;
  switch (code) {
    int x;
    int *p;
  case 0:
    p = &x;
    *p = 5;
    total = x;
    break;
  default:
    total = 1;
  }
  return total;
}
