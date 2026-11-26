// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (306; the pin struct_array_reject_annotations.c's through_load_read, with the atom that frames the read): p->q->a[i] with a symbolic index in a contract, framed by reads(*p->q): the callee returns it and states it
// Pre-registered from record 0064 item 15 and the precedent struct_chain_whole.c, before any run: verifies (unmeasured: a load-rooted element read with a symbolic index)
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

int get(struct R *p, int i)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*p->q) && 0 <= i && i < 4)
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*p->q) && r == p->q->a[i])
{
  return p->q->a[i];
}

int use(void)
    CERASTES_ENSURES(r, r <= 2147483647)
{
  struct S t = {{1, 2, 3, 4}, 5};
  struct R s = {&t, 1};
  return get(&s, 2);
}
