// expect: error permission @ L41
// expect: error permission @ L39
// expect: error permission @ L39
// expect: error invalid-free @ L42
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_main_holds.c
// Pins: 0056 item 5 (a false alarm pinned, the program is correct; audit round 1 S2): main reaches the struct global's leaf g.data through the alias pg = &g in the loop, so hs (held and the cell has permission, taken at the loop entry) is false at the entry and the callee's gives-back raises held in the body: the row !hs ==> !held fails at the back edge. The limit of the entry snapshot; main_holds shows the loop that does not name g through an alias verifies
// Pre-registered from audit round 1 S2 and record 0056 item 5 (not yet run on the L-2 build): a mapped permission at the closing brace of the loop body (the back-edge row !hs ==> !held), in both modes, exit status 1; never an exit 3 [Corrected after the first run on the L-2 build (question 160): a pin records what the encoder reports; besides the designed permission at the loop's closing brace (the row !hs ==> !held) it reports the refill call (permission x2: the callee needs the whole allocation, and `p->data` and `*pg` may be the same object) and the free after the loop (invalid-free: main does not hold g.data by the entry snapshot).]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void refill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 1;
  }
}

int main(void)
{
  g.data = NULL;
  g.n = 0;
  struct buf *pg = &g;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    refill(pg);
    i = i + 1;
  }
  free(g.data);
  g.data = NULL;
  return 0;
}
