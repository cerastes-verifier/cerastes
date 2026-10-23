// expect: rejected cerastes-unsequenced_modification @ L28
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021 and 0020 item 4: a call that reads x next to a store of x is still an unsequenced modification, whatever the calls' sets are
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int g(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int h(int z)
    CERASTES_ENSURES(r, r == z)
{
  return z;
}

int test(int a)
{
  int x;
  return (x = f(a)) + g(x);
}
