// expect: error permission @ L38
// expect: error permission @ L42
// expect: error uninitialized @ L46
// expect: error lifetime @ L50
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_escape_none.c
// Pins: 0056 item 11 (question 209, the escape scan), route passed to a call: `struct buf *ps = same(&s);` where the callee returns its argument: the struct local's address escapes, so the loop reaches s.data (NULL at the entry) and fills it through the pointer with a block it never writes; the leaf's flag is not kept and the read after the loop is uninitialized (the designed error), beside the loop's own row (permission: the loop needs owning field s.data to point to a block the function holds, or to be null) that the store through an alias gets today
// Pre-registered from record 0056 item 11 and audit round 2 S2: uninitialized at the read after the loop; the expect lines are the outcome of the L-1 build, where every struct local is reached by type, which the escape scan must keep for a local whose address escapes (any change when 209 is built is a stop) [Corrected after the first run on the build with 208-211, 213 (question 160): in c++20 an error of the loop's entry row is reported at the first attribute line of the loop, in c23 at the `while`, so the attributes and the `while` now stand on one line (every later line moved up by two).]
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

struct buf *same(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*r) && r == p)
{
  return p;
}

int f(void)
{
  struct buf s;
  s.n = 0;
  s.data = NULL;
  struct buf *ps = same(&s);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3) CERASTES_DECREASES(3 - i) while (i < 3) {
    free(ps->data);
    ps->data = (int *)malloc(sizeof(int));
    i = tick(i);
  }
  ps = NULL;
  int x = 0;
  if (s.data != NULL) {
    x = *s.data;
  }
  free(s.data);
  s.data = NULL;
  return x;
}
