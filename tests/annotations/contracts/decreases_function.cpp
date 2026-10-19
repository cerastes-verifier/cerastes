// expect: golden
// expect: native-ok
// Pins: decision 0013 item 1: decreases after a function's parameter list, in any order with requires and ensures; several measures; a member
#include <cerastes.h>


int count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return count_down(n - 1);
}

int decreases_first(int n) CERASTES_DECREASES(n) CERASTES_REQUIRES(n >= 0);

int lexicographic(int m, int n)
    CERASTES_REQUIRES(m >= 0 && n >= 0)
    CERASTES_DECREASES(m, n)
{
  if (m == 0) {
    return n;
  }
  return lexicographic(m - 1, n);
}

struct counter {
  int down(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES(n);
};

int counter::down(int n) CERASTES_REQUIRES(n >= 0) CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : down(n - 1);
}
