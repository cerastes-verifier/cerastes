// expect: error uninitialized @ L37
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_site_store_written.c
// Pins: 0056 item 2: a struct local's owning leaf s.data holds a malloc'd block that is never written, a loop that stores a pointer of the leaf's type through memory (`*pz = pk;`) does not touch it, and the block is returned after the loop: uninitialized at the return. This twin does not pin the route of item 2: the block is there before the loop, so the flag is false at the entry whatever wef is (audit round 2 B1; the models owning_field_loop_site_store_null_unwritten.c and owning_field_loop_site_addr_null_unwritten.c pin the routes); it is the error twin of the returned-block loop
// Pre-registered from record 0056 item 2 and audit S4 (designed outcome; build/m4.5d2b2c/t5/r1.c on the L-1 build before filing): uninitialized at the return of s.data, in both modes
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

CERASTES_OWNING int *f(void)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  int k = 0;
  int *pk = &k;
  int *z = NULL;
  int **pz = &z;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *pz = pk;
    i = tick(i);
  }
  return s.data;
}
