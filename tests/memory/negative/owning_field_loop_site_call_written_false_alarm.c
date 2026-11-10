// expect: error uninitialized @ L50
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_pointer_arg.c
// Pins: 0056 item 2 (a false alarm pinned: the program is correct): the same function as owning_field_loop_site_call_unwritten.c with the block of s.data written before the loop; the route a call whose ensures has a block site through a load of the site's pointer type (`give(&z)`, ensures `*pp == NULL || owns_block(*pp)`) is in the loop, so wef is false and the flag of the written block is not kept: the return reports a block that has a value. A loop that may put a pointer of the leaf's type into memory, or take it from a callee, is treated as possibly filling the leaf (the sound direction of item 2); the written-invariant rule of item 8 is the way out
// Pre-registered from audit S4 as the neighbour of the unwritten twin (build/m4.5d2b2c/t5/r4w.c on the L-1 build): uninitialized at the return, in both modes
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

void give(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && *pp == NULL)
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp != NULL) {
    **pp = 0;
  }
}

CERASTES_OWNING int *f(void)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 1;
  }
  int *z CERASTES_OWNING = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(z == NULL)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    give(&z);
    free(z);
    z = NULL;
    i = tick(i);
  }
  return s.data;
}
