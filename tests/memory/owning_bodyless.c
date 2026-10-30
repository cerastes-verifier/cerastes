// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 3 (audit S6 and the orchestrator's note): an owning parameter is a contract: a body-less callee with only the attribute is callable, and gives the cell back
#include <cerastes.h>
#include <stddef.h>

void put(int *p CERASTES_OWNING);

int use_put(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 0;
  put(&x);
  put(NULL);
  x = 3;
  return x;
}
