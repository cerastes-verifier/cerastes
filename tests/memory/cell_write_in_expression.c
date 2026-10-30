// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2: an assignment to a cell used as a value: an argument (f(x = 1)), an operand through a pointer ((*p = 3) + 1), a chain (x = *q = 5); each stores once and its value is the stored value
#include <cerastes.h>

int take(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int in_expression(void)
    CERASTES_ENSURES(r, r == 18)
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
