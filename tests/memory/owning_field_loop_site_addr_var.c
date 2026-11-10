// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 2, audit round 1 S4: the route of loop_may_fill that is a write of an address-taken variable of the site's pointer type (`z = pk;` with `&z` taken): a loop that has it, beside a struct local's owning leaf s.data that the loop does not name, with the block of s.data written before the loop: the flag survives the loop and the block is returned
// Pre-registered from record 0056 item 2 and audit S4 (build/m4.5d2b2c/t5/r3w.c on the L-1 build before filing): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

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

CERASTES_OWNING int *f(void)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 1;
  }
  int k = 0;
  int *pk = &k;
  int *z = NULL;
  int **pz = &z;
  keep(pz);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    z = pk;
    i = tick(i);
  }
  return s.data;
}
