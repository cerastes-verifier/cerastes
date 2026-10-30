// expect: error postcondition @ L26
// mode: c23, c++20
// twin: tests/memory/cell_write_in_expression.c
// Pins: 0039 item 2: the same program with the sum written as 19: the stored values are 1, 4 (3 + 1), 5, 3 and 5, so the postcondition fails at the return
#include <cerastes.h>

int take(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int in_expression(void)
    CERASTES_ENSURES(r, r == 19)
{
  int x = 0;
  int y = 0;
  int z = 0;
  int *px = &x;
  int *p = &y;
  int *q = &z;
  int a = take(x = 1);
  int b = (*p = 3) + 1;
  x = *q = 5;
  CERASTES_CHECK(a == 1 && b == 4 && *px == 5 && y == 3 && z == 5);
  return a + b + *px + y + z;
}
