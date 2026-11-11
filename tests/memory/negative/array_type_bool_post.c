// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/array_type_bool.c
// Pins: 0059 item 2: a[1] was given a[0], which is true
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int flags(void)
    CERASTES_ENSURES(r, r == 0)
{
  bool a[3] = {true, false, true};
  a[1] = a[0];
  return a[1] ? 1 : 0;
}
