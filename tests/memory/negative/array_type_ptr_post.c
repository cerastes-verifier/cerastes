// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/array_type_ptr.c
// Pins: 0059 item 2: x was written through ps[0]
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int through(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int *ps[2] = {&x, &x};
  *ps[0] = 1;
  return x;
}
