// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit S3, j_parked_inner.c): a borrowed block parked in an owning local of an inner block: the entry value of the formal with an ensures block site is an owner at the inner block's end too, so no leak is reported there (and the caller writes the block before the call)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) S3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void touch(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  int k = 0;
  {
    CERASTES_OWNING int *v = p;
    *v = 1;
  }
  k = 1;
  *p = k;
}

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 0;
  touch(p);
  free(p);
  return 0;
}
