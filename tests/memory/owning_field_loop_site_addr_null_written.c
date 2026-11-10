// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 2, audit round 2 B1 (address-taken variable route): the model of owning_field_loop_site_addr_null_unwritten.c with the block written in the loop
// Pre-registered from record 0056 item 2 (probe build/m4.5d2b2c/t7/addr_w.c on the L-1 build): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

void keep(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp))
{
  (void)pp;
}

int f(void)
{
  int *v CERASTES_OWNING = NULL;
  int **pv = &v;
  keep(pv);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(v);
    v = (int *)malloc(sizeof(int));
    if (v != NULL) {
      *v = i;
    }
    i = tick(i);
  }
  int x = 0;
  if (v != NULL) {
    x = *v;
  }
  free(v);
  v = NULL;
  return x;
}
