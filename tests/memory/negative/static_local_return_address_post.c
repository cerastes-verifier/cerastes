// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/static_local_return_address.c
// Pins: 0041 item 12: the address of a static local is never null: r == NULL fails
#include <cerastes.h>
#include <stddef.h>

int *where(void)
    CERASTES_ENSURES(r, r == NULL)
{
  static int s = 5;
  return &s;
}
