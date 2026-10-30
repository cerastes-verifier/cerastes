// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 3: owning on a T** parameter owns the pointer cell *pp
#include <cerastes.h>
#include <stddef.h>

void clear(CERASTES_OWNING int **pp)
{
  if (pp != NULL) {
    *pp = NULL;
  }
}

int use_clear(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  int *p = &x;
  clear(&p);
  clear(NULL);
  return x;
}
