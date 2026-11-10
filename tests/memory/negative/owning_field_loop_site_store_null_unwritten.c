// expect: error uninitialized @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_site_store_null_written.c
// Pins: 0056 item 2, audit round 2 B1 (store route): a struct local's owning leaf s.data is NULL at the loop's entry and the loop fills it by a non-null store (`s.data = malloc(...)`) with a block it never writes: the route is seen, so wef is false and the read after the loop is uninitialized. A scan that drops the route makes wef true and moves the error to the loop's back-edge row (an iteration may end with s.data having no value), on the line of the store's loop
// Pre-registered from the auditor's model (build/m4.5d2b2c/t7/model_store.c, run on the L-1 build): uninitialized at the read `x = *s.data;` in both modes; with wef flipped by hand the error moved to the back-edge row (auditor, round 2 B1)
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

int f(void)
{
  struct buf s;
  s.n = 0;
  s.data = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(s.data);
    s.data = (int *)malloc(sizeof(int));
    i = tick(i);
  }
  int x = 0;
  if (s.data != NULL) {
    x = *s.data;
  }
  free(s.data);
  s.data = NULL;
  return x;
}
