// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3: an owning local handed to a callee that takes the block (owns_block in its requires alone): the caller no longer holds it at its scope exit
// Pre-registered from record 0052 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
  free(p);
}

int make_and_dispose(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 5;
  dispose(p);
  return 0;
}
