// expect: golden
// expect: native-ok
// Pins: decision 0006 item 6 (must match) for ensures: a result name is part of
// the contract's shape but its spelling is not (the binder is compared by
// position, like a parameter); an ensures without a name matches only an
// ensures without a name. Twin of ensures_unnamed_then_named and
// ensures_named_then_unnamed.
#include <cerastes.h>

int f(int x) CERASTES_ENSURES(r, r > x);

int f(int y) CERASTES_ENSURES(res, res > y)
{
  return y + 1;
}

int g(int x) CERASTES_ENSURES(x < 100);

int g(int y) CERASTES_ENSURES(y < 100)
{
  return y;
}
