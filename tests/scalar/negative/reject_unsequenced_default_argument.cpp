// expect: rejected cerastes-unsequenced_calls @ L26
// twin: tests/scalar/call_reorder.c
// Pins: 0021 item 3: a call in a default argument belongs to the call that uses the default: the error is at that call, not at the declaration of the default
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int z)
    CERASTES_ENSURES(r, r == z)
{
  return z;
}

int dflt(int a, int b = h(1))
    CERASTES_ENSURES(r, r == a)
{
  return a + (b - b);
}

int test(int x, int y)
{
  return 100 / f(x) + 100 / dflt(y);
}
