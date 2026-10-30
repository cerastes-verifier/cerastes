// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 3 (question 75): owning beside owns(*p) (p is non-null now) and beside p == NULL || owns(*p) is merged into one atom with the explicit guard
#include <cerastes.h>
#include <stddef.h>

void set_cell(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
{
  *p = 1;
}

void set_maybe(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
{
  if (p != NULL) {
    *p = 2;
  }
}

int use_merged(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 0;
  set_cell(&x);
  set_maybe(&x);
  set_maybe(NULL);
  x = 3;
  return x;
}
