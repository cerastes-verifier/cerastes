// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/scalar/enum_loop.c
// Pins: 0018 item 4: with 0 <= i in place of the link between c and i the loop proves only that c is a value of enum color
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, (int)r == n)
{
  enum color c = RED;
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_INVARIANT(0 <= i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    c = (enum color)(i + 1);
    i += 1;
  }
  return c;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  enum color a = count_up(3);
  enum color b = count_up(2);
  enum color z = count_up(0);
  return (int)a + (int)b + (int)z;
}
