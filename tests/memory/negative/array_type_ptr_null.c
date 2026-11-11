// expect: error null-dereference @ L15
// mode: c23, c++20
// twin: tests/memory/array_type_ptr.c
// Pins: 0059 item 2: an element written as NULL is null: null-dereference at the *
// Pre-registered from record 0059 item 2
#include <cerastes.h>

#include <stddef.h>

int through(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *ps[2] = {&x, NULL};
  *ps[1] = 1;
  return x;
}
