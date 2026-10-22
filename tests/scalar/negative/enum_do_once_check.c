// expect: error check @ L16
// mode: c23, c++20
// twin: tests/scalar/enum_do_once.c
// Pins: 0018 item 9: the loop body runs once, so i is n + 1 there, not n
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int once(int n)
    CERASTES_REQUIRES(n < 2147483647)
    CERASTES_ENSURES(r, r == n + 1)
{
  int i = n;
  do {
    i += 1;
    CERASTES_CHECK(i == n);
  } while (RED);
  return i;
}

int use(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return once(2147483646);
}
