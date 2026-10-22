// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 4: an enumeration local keeps its value range through a loop (the automatic range invariant), and the invariant can name it
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, (int)r == n)
{
  enum color c = RED;
  int i = 0;
  CERASTES_INVARIANT(i <= n)
  CERASTES_INVARIANT((int)c == i)
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
