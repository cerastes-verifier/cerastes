// expect: verified
// expect: native-ok
// mode: c23, c++20
// A cell written inside an expression: an argument, an operand through a pointer, and a postfix increment whose value is the old one.
// tag::source[]
#include <cerastes.h>

int take(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int in_expression(void)
    CERASTES_ENSURES(r, r == 11)
{
  int x = 0;
  int y = 0;
  int *px = &x;
  int *p = &y;
  int a = take(x = 1);
  int b = (*p = 3) + 1;
  int c = x++;
  return a + b + c + *px + y;
}
// end::source[]
