// expect: golden
// expect: native-ok
// Pins: decision 0006 item 6: the usual style: the contract on the header
// declaration only, a definition with no contract; the function has the
// contract (a declaration without one has the function's contract).
#include <cerastes.h>

int halve(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r <= x);

int halve(int x)
{
  return x / 2;
}
