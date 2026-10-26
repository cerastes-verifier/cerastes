// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 2 (question 28): writes and snapshots of call results leave the chain, so these are accepted; each check and each call keeps its place
#include <cerastes.h>

int capped(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == (x > 1000 ? 1000 : x));

int small(int y)
    CERASTES_REQUIRES(y >= 0)
    CERASTES_ENSURES(r, r == (y > 500 ? 500 : y));

// the stores of two calls' results in one sum: both orders give x and y the same values
int two_writes(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == 2 * ((a > 1000 ? 1000 : a) + (b > 500 ? 500 : b)))
{
  int x;
  int y;
  int s = (x = capped(a)) + (y = small(b));
  return s + x + y;
}

// a call and a store of another call's result
int call_then_write(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == (b > 1000 ? 1000 : b) + 2 * (a > 500 ? 500 : a))
{
  int x;
  int s = capped(b) + (x = small(a));
  return s + x;
}

// the value of a store read back with a comma: the locals are initialized, so no check follows
int snapshots(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == (a > 1000 ? 1000 : a) + (b > 500 ? 500 : b))
{
  int x = 0;
  int y = 0;
  int s = (x = capped(a), x) + (y = small(b), y);
  return s;
}
