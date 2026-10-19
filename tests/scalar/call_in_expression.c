// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 6: calls nested in one another are evaluated innermost first and each is checked
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r >= 0 && r <= 1000)
{
  return x > 1000 ? 1000 : x;
}

int chain(int a)
    CERASTES_REQUIRES(a >= 0)
    CERASTES_ENSURES(r, r >= 1000 && r <= 2000)
{
  return bounded(bounded(a)) + 1000;
}
