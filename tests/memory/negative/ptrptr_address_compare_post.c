// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_address_compare.c
// Pins: 0040 item 2: pp holds the address of p, so the comparison is true
#include <cerastes.h>

int address_of_pointer(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  return pp == &p;
}
