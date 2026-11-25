// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 9 and 10: a structure global with an array member in main (its initial values known, a read-only loop over g.a that keeps the values) and a static structure local with an array member and its implicit atom
// Pre-registered from record 0063 item 9 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

static struct S g = {{1, 2, 3, 4}, 5};

int third(void)
    CERASTES_ENSURES(r, r == 3)
{
  static struct S c = {{7, 7, 7, 7}, 1};
  c.a[1] = 3;
  return c.a[1];
}

int main(void)
{
  int s = 0;
  int i = 0;
  CERASTES_CHECK(g.a[2] == 3 && g.n == 5);
  CERASTES_INVARIANT(0 <= i && i <= 4 && 0 <= s && s <= 4 * i)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    s += g.a[i];
    i += 1;
  }
  CERASTES_CHECK(s <= 16);
  return s - s;
}
