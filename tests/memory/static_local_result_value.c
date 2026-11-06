// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (audit B5): ensures(r, owns(*r) && *r >= 0) on a function returning &s of a static local: the implicit owns(s) is emitted before the written clauses, so the cover frames the read *r in the same ensures; the caller learns the bound
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r) && *r >= 0)
{
  static int s = 5;
  s = 5;
  return &s;
}

int main(void)
{
  int *p = where();
  int v = *p;
  CERASTES_CHECK(v >= 0);
  return v >= 0 ? 0 : 1;
}
