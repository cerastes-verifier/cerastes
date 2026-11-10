// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 2 (question 208): the block of s.data is written before a loop that calls a callee whose ensures has a block site through a load of the leaf's pointer type: the site is plain (every cell's flag is set), so the call can give no unwritten block and does not count as a way to fill the leaf: the flag of the written block is kept
// Pre-registered from audit round 2 S1 and question 208 (the former pin owning_field_loop_site_call_written_false_alarm.c, with the plain call route restricted away; not yet run on the build with 208): verifies in both modes
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
