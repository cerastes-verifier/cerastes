// expect: error uninitialized @ L47
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_site_call_written.c
// Pins: 0056 item 2 (question 208): the same function without the write of s.data's block: the leaf's block has no value at the loop's entry, and the plain call does not make the leaf fillable, so wef is true and the loop's entry row reports it, mapped at the loop (question 160 applies to the line: the loop's head or its first invariant)
// Pre-registered from record 0056 item 2 and question 208 (not yet run on the build with 208): uninitialized at the loop's entry row, in both modes; the base's error was at the return, which the route's removal moves [Corrected after the first run on the build with 208-211, 213 (question 160): the entry row does not fire for this program; the unwritten block is reported where it is returned (L47, `return s.data;`), as for the base: same category, a later line.]
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
