// expect: rejected cerastes-self_assignment @ L11
// expect: rejected cerastes-self_assignment @ L17
// expect: rejected cerastes-self_assignment @ L23
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): the inner assignment of y = (x = x);, y = x = x; and x = (x = x); is a self-assignment (the outer one of the first two is not)
#include <cerastes.h>

int nested_parens(int x, int y)
{
  y = (x = x);
  return x + y;
}

int chained(int x, int y)
{
  y = x = x;
  return x + y;
}

int outer_too(int x)
{
  x = (x = x);
  return x;
}
