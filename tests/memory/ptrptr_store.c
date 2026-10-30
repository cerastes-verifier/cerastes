// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2: a pointer variable whose address is taken is a cell; **pp = 5 writes x through the cell p and the loaded pointer
#include <cerastes.h>
#include <stddef.h>

int store_through(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  **pp = 5;
  CERASTES_CHECK(x == 5);
  return x;
}
