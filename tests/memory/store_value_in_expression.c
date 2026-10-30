// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2 (audit B3): int y = (*p = 1); through a pointer the function owns: the null check and the write permission hold, the value is the stored one
#include <cerastes.h>
#include <stddef.h>

int store_value(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == 1 && r == 1)
{
  int y = (*p = 1);
  return y;
}
