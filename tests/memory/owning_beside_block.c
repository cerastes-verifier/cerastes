// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 6c (the audit N7 case): cerastes::owning on a parameter beside owns_block of the same parameter in both lists: merged into the block site, which gives everything owns(*p) does
// Pre-registered from record 0052 item 6c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void touch(CERASTES_OWNING int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  if (p != NULL) {
    *p = 1;
  }
}

int use_touch(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  touch(p);
  free(p);
  return 0;
}
