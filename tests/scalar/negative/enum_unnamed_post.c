// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/enum_unnamed.c
// Pins: 0018 items 1 and 9: pick(U1) returns 12, but the ensures says x + 10
#include <cerastes.h>

enum { U0, U1 };

int pick(int x)
    CERASTES_REQUIRES(x == U0 || x == U1)
    CERASTES_ENSURES(r, r == x + 10)
{
  if (x == U0) {
    return 10;
  }
  return 12;
}

int both(void)
    CERASTES_ENSURES(r, r == 21)
{
  int a = pick(U0);
  int b = pick(U1);
  return a + b;
}

int use(int y)
    CERASTES_REQUIRES(y == U0 || y == U1)
    CERASTES_ENSURES(r, r == y + 10)
{
  return pick(y);
}

int via_use(void)
    CERASTES_ENSURES(r, r == 21)
{
  int a = use(U0);
  int b = use(U1);
  return a + b;
}
