// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0013 item 4: a function outside every recursion cycle needs no measure, and may call one that has
#include <cerastes.h>

int countdown_leaf(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return countdown_leaf(n - 1);
}

int plain_caller(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == 0)
{
  return countdown_leaf(n);
}
