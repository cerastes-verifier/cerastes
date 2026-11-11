// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/array_type_ptr_ptr.c
// Pins: 0059 item 2: x was written through the element
// Pre-registered from the M4.5e1a sweep (report 5, x05)
#include <cerastes.h>

int through(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int *p = &x;
  int **pp[1] = {&p};
  **pp[0] = 1;
  return x;
}
