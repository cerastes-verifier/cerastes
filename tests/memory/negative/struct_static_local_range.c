// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/struct_static_local.c
// Pins: 0044 item 9: the member c.x is unknown on entry, so next without the reset can return 101: postcondition at the return
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int next(void)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  static struct S c;
  if (c.x < 0 || c.x > 100) {
    c.x = 0;
  }
  c.x += 1;
  return c.x;  // <- here
}

int twice_in_order(void)
    CERASTES_ENSURES(r, 2 <= r && r <= 200)
{
  int a = next();
  int b = next();
  return a + b;
}

void ticks(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_DECREASES(n)
{
  if (n > 0) {
    next();
    ticks(n - 1);
  }
}

int main(void)
{
  int a = next();
  CERASTES_CHECK(1 <= a && a <= 100);
  ticks(3);
  int x = twice_in_order();
  return a + x;
}
