// expect: golden
// expect: native-ok
// Pins: decision 0013 item 1: decreases before a while loop, next to invariant (a one-component and a lexicographic measure)
#include <cerastes.h>


int count_up(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    i += 1;
  }
  return i;
}

int count_up_pair(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_DECREASES(n - i, 5) while (i < n) {
    i += 1;
  }
  return i;
}
