// expect: error null-dereference @ L13
// mode: c23, c++20
// twin: tests/memory/array_type_ptr.c
// Pins: 0059 item 3: an element without an initializer is a null pointer (zero_arr_ptr's zero value)
// Pre-registered from record 0059 item 3
#include <cerastes.h>

int through(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *ps[2] = {&x};
  *ps[1] = 1;
  return x;
}
