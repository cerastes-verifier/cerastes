// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 4: owning beside must_init: the owning atom (p == NULL || owns(*p)) carries the marker; the callee initializes *p where p is not null; both positions
#include <cerastes.h>
#include <stddef.h>

void set_zero(CERASTES_OWNING CERASTES_MUST_INIT int *p)
    CERASTES_ENSURES(p == NULL || *p == 0)
{
  if (p != NULL) {
    *p = 0;
  }
}

void set_zero_trailing(int *p CERASTES_OWNING CERASTES_MUST_INIT)
    CERASTES_ENSURES(p == NULL || *p == 0)
{
  if (p != NULL) {
    *p = 0;
  }
}

int use_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  set_zero(&x);
  int y;
  set_zero_trailing(&y);
  return x + y;
}
