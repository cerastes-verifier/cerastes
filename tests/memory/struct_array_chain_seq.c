// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 and handoff section 3 (the sequencing rows of a call on a place reached through a heap read): the call that owns *p->q, then a read in the next statement; and a reads call beside a read of an element through the same place, both accepted. The twins in negative/struct_array_chain_seq_unsequenced.c put the read, a second call, or the same object by name into the expression
// Pre-registered from record 0064 item 11 and item 15 (Part D), before any run: verifies (unmeasured)
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

int wr(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && p->q->a[1] == 7 && r == 0)
{
  p->q->a[1] = 7;
  return 0;
}

int rd(struct R *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*p->q))
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*p->q) && (r == 0 || r == 1))
{
  return p->q->a[1] == p->q->a[2];
}

int sequenced(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S t = {{1, 2, 3, 4}, 5};
  struct R s = {&t, 1};
  int k = wr(&s);
  CERASTES_CHECK(t.a[1] == 7);
  return k;
}

int reads_beside(void)
    CERASTES_ENSURES(r, r == 0 || r == 1)
{
  struct S t = {{1, 2, 3, 4}, 5};
  struct R s = {&t, 1};
  return rd(&s) != s.q->a[1];
}
