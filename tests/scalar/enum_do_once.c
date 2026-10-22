// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0018 item 9: an enumerator names no object, so do ... while (RED) with RED == 0 runs once and needs no decreases, in both modes
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int once(int n)
    CERASTES_REQUIRES(n < 2147483647)
    CERASTES_ENSURES(r, r == n + 1)
{
  int i = n;
  do {
    i += 1;
    CERASTES_CHECK(i == n + 1);
  } while (RED);
  return i;
}

int use(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return once(2147483646);
}
