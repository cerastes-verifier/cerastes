// expect: verified
// expect: native-ok
// mode: c23, c++20
// A const global used in a contract and in the program, and a switch whose cases are constant expressions.
// tag::source[]
#include <cerastes.h>

const int MAX_LEVEL = 3;

int level_cost(int level)
    CERASTES_REQUIRES(0 <= level && level <= MAX_LEVEL)
    CERASTES_ENSURES(r, r == 10 * level)
{
  switch (level) {
  case 0:
    return 0;
  case 1:
    return 10;
  case 1 << 1:
    return 20;
  default:
    return 30;
  }
}

int extremes(void)
    CERASTES_ENSURES(r, r == 30)
{
  int cheapest = level_cost(0);
  int dearest = level_cost(MAX_LEVEL);
  return cheapest + dearest;
}
// end::source[]
