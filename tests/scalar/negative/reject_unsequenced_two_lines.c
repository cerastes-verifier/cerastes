// expect: rejected cerastes-unsequenced_calls @ L22
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021 item 3: the two conflicting calls on separate lines: the error is at the later-created call (g), on the second line
#include <cerastes.h>

int g(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int test(int x, int y)
{
  return 100 / g(x) +
         100 / h(y);
}
