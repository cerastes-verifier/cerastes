// expect: error unsequenced @ L14
// mode: c23, c++20
// twin: tests/memory/cell_pointer_store_postfix.c
// Pins: 0039 item 3: q = c ? p : &y lets *p = (*q)++ name one object on the path c != 0: the obligation p != q fails there (path-precise, P61 B: no compiler warns)
#include <cerastes.h>

int copy_inc(int c)
    CERASTES_ENSURES(r, r >= 0 || r < 0)
{
  int x = 1;
  int y = 5;
  int *p = &x;
  int *q = c ? p : &y;
  *p = (*q)++;
  return x;
}
