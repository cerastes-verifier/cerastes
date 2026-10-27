// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 item 6 (this replaces the rejection of 0021 by reject_unsequenced_split_requires.c): take(f(x), h(y)) with one requires per argument; both clauses wait for both calls, and the caller's requires makes them hold
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int take(int a, int b)
    CERASTES_REQUIRES(1 <= a)
    CERASTES_REQUIRES(2 <= b)
    CERASTES_ENSURES(r, r == a - b)
{
  return a - b;
}

int test(int x, int y)
    CERASTES_REQUIRES(1 <= x && 2 <= y)
    CERASTES_ENSURES(r, r == x - y)
{
  return take(f(x), h(y));
}
