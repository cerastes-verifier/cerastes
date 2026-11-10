// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2: the plain int *p form: the marker on a pointer to a cell whose contract returns what it takes (owns(*p) in and out); no loaded pointer lies on the place's path, so the implicit ensures is empty and the written contract speaks about the value
// Pre-registered from record 0058 item 2: verifies in both modes
#include <cerastes.h>

void bump(int *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  bump(&x);
  return x;
}
