// expect: error leak @ L31
// expect: error permission @ L31
// expect: error invalid-free @ L32
// expect: error leak @ L33
// mode: c23, c++20
// twin: tests/memory/marked_pointee_param.c
// Pins: 0058 item 4 (audit round 3 N3, PINNED FALSE ALARM): a loop that calls a ref_to_uninit initializer of a struct local whose owning field holds a block: the program keeps t = s.q, calls init(&s) three times, frees t and nulls s.q: correct C, but the marked site closes nothing, so the loop cannot carry the current value of the leaf (the limit of 0054 item 3 and 0056): the back edge reports a leak and a permission error, the free of t an invalid-free and the store a leak; sound, listed with the price of question 202, to lift with the current-value carriers
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: the four errors of the auditor's probe (audit3/q3_loop_ok.c: back-edge leak and permission, invalid-free on t, a store leak); the multiset and the lines are confirmed or corrected at the first run (question 160)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *q CERASTES_OWNING; int n; };
void init(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->q = NULL;
  p->n = 0;
}
int use(void)
{
  struct buf s;
  s.q = (int *)malloc(sizeof(int));
  s.n = 1;
  int *t = s.q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    init(&s);
    i = i + 1;
  }  // <- here
  free(t);  // <- here
  s.q = NULL;  // <- here
  return 0;
}
