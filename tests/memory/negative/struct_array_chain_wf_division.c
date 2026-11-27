// expect: error division-by-zero @ L23
// expect: error division-by-zero @ L31
// mode: c23, c++20
// twin: tests/memory/struct_array_chain_wf.c
// Pins: 0064 item 7 (f__wf's ghosts, one per leaf, slot and loaded pointer, shared by no two): each function guards p->q->a[1] != 0 and divides by another place of the same loaded structure in the same clause. other_slot: p->q->a[2]; other_leaf: p->q->n. A shared ghost name would prove each of them. The two-pointer form (p->q2->a[1], the other_root row) is held for the owner's decision on a lint rule (question 357)
// Pre-registered from record 0064 items 7 and 15 and the precedent negative/range_struct_wf_division.c, before any run: division-by-zero at the ensures line of each function (unmeasured for a load-rooted place)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct R {
  struct S *q;
  int z;
};

int other_slot(struct R *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*p->q) && p->q->a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*p->q) && p->q->a[1] != 0 && (r == 0 || r == 10 / p->q->a[2]))  // <- here
{
  (void)p;
  return 0;
}

int other_leaf(struct R *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*p->q) && p->q->a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*p->q) && p->q->a[1] != 0 && (r == 0 || r == 10 / p->q->n))  // <- here
{
  (void)p;
  return 0;
}
