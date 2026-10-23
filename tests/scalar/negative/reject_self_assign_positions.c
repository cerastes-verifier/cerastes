// expect: rejected cerastes-self_assignment @ L17
// expect: rejected cerastes-self_assignment @ L22
// expect: rejected cerastes-self_assignment @ L27
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): the position of the assignment does not matter: an argument, a return value and an arm of ?:
#include <cerastes.h>

int id(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int as_argument(int x)
{
  return id(x = x);
}

int as_return_value(int x)
{
  return x = x;
}

int in_arm(int c, int x)
{
  return c ? (x = x) : 0;
}
