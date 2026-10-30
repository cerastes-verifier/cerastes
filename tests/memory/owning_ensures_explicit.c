// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 3 (audit S7): owning beside an explicit ensures(owns(*p)); the requires says p != NULL, so the ensures atom has its requires partner on every path
#include <cerastes.h>
#include <stddef.h>

void touch(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(p != NULL)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 1;
}
