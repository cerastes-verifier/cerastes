// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/memory/ptrptr_null.c
// Pins: 0040 item 2: the null pointer to pointer compares equal to null
#include <cerastes.h>

int pointer_to_pointer(void)
    CERASTES_ENSURES(r, r == 0)
{
  int **pp = 0;
  return pp == 0;
}
