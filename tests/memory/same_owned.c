// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 item 12: == of two pointers that own their cells (offset < len, no one-past-the-end) is fine; the contract's == is model identity
#include <cerastes.h>
#include <stddef.h>

int same(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == (p == q))
{
  return p == q;
}

int use_same(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  int y = 2;
  return same(&x, &y);
}
