// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 4: must_init on T **pp covers the pointer cell *pp (its range is WF); the caller reads q after make(&q)
#include <cerastes.h>
#include <stddef.h>

void make(int **pp CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == NULL)
{
  *pp = NULL;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 1)
{
  int *q;
  make(&q);
  if (q == NULL) {
    return 1;
  }
  return 0;
}
