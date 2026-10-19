// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0010 item 12: sizeof(int) is 4 on this target, not 8
#include <cerastes.h>

int int_size(void)
    CERASTES_ENSURES(r, r == 8)
{
  return (int)sizeof(int);
}
