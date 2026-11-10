// expect: error permission @ L35
// expect: error uninitialized @ L38
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_escape_none.c
// Pins: 0056 item 11 (question 209, the escape scan), route stored into a pointer variable: `struct buf *ps = &s;`: the struct local's address escapes, so the loop reaches s.data (NULL at the entry) and fills it through the pointer with a block it never writes; the leaf's flag is not kept and the read after the loop is uninitialized (the designed error), beside the loop's own row (permission: the loop needs owning field s.data to point to a block the function holds, or to be null) that the store through an alias gets today
// Pre-registered from record 0056 item 11 and audit round 2 S2: uninitialized at the read after the loop; the expect lines are the outcome of the L-1 build, where every struct local is reached by type, which the escape scan must keep for a local whose address escapes (any change when 209 is built is a stop)
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
  struct buf *ps = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(ps->data);
    ps->data = (int *)malloc(sizeof(int));
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
