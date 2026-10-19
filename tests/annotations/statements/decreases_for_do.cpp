// expect: golden
// expect: native-ok
// Pins: decision 0013 item 1: decreases before for and do loops (the counter is declared before the loop, as for invariant)
#include <cerastes.h>


int count(int n) CERASTES_REQUIRES(n >= 0)
{
  int i;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == i) CERASTES_DECREASES(n - i) for (i = 0; i < n; ++i) {
    c += 1;
  }
  return c;
}

int at_least_one(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_DECREASES(n + 1 - i) do {
    i += 1;
  } while (i < n);
  return i;
}
