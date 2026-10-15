// expect: golden
// expect: native-ok
// Pins: ensures(p) with no result name; the parameter denotes its entry value
// even though the body assigns to it (decision 0006 item 5).
#include <cerastes.h>

int next(int x) CERASTES_ENSURES(x < 1000)
{
  x = x + 1;
  return x;
}
