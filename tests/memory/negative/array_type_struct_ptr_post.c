// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/array_type_struct_ptr.c
// Pins: 0059 item 2: s.x was written through the element
// Pre-registered from the M4.5e1a sweep (report 5, x03)
#include <cerastes.h>

struct S {
  int x;
};

int through(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s;
  s.x = 0;
  struct S *arr[1] = {&s};
  arr[0]->x = 1;
  return s.x;
}
