// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/array_type_enum.c
// Pins: 0059 item 2: a[1] is BLUE, 2
// Pre-registered from record 0059 item 2
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int third(void)
    CERASTES_ENSURES(r, r == 1)
{
  enum color a[2] = {RED, BLUE};
  return (int)a[1];
}
