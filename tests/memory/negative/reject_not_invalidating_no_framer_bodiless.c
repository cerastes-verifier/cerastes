// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/not_invalidating_borrower.c
// Pins: 0058 item 2 (audit S1): the requires takes owns_block(p->q) through the marked p, and the only ensures atom that could frame the load of p->q is rooted at p2 (p == p2 is in the requires): no ensures atom or site rooted at p frames it, so the implicit p->q == old(p->q) cannot be placed: unsupported_annotation at the parameter (bodiless). Without the marker the program verifies (build/m4.5d2b2d/emul3/reject_not_invalidating_no_framer_bodiless_plain.c)
// Pre-registered from record 0058 item 2 (audit round 1, S1; orchestrator, task 6): rejected unsupported_annotation at the parameter, "cannot place p->q == old(p->q): no ensures atom on p frames it"
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *q;
  int n;
};

void touch(struct buf *p CERASTES_NOT_INVALIDATING, struct buf *p2)  // <- here
    CERASTES_REQUIRES(p == p2 && cerastes_owns(*p2) && cerastes_owns_block(p->q))
    CERASTES_ENSURES(cerastes_owns(*p2) && cerastes_owns_block(p->q));
