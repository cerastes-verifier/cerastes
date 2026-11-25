// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 7 and 8 (audit round 3 S2, the probe a13j): two readers of one slot: reads(*p) and reads(*q) with q the address of s.a[1] hold together (the call's per-slot amounts add up to at most the whole permission); the lint has no `perm` under a quantifier in the call's rows (twin negative/struct_array_reads_owns_alias.c)
// Pre-registered from audit round 3 S2 and record 0063 item 7 (Part C), before any run: verifies (the call row today carries a `perm` under a quantifier, which the lint refuses)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void two(struct S *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_reads(*q))
    CERASTES_ENSURES(cerastes_reads(*p) && cerastes_reads(*q))
{
  (void)p;
  (void)q;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{1, 2, 3, 4}, 5};
  two(&s, &s.a[1]);
  CERASTES_CHECK(s.a[1] == 2 && s.n == 5);
  return s.n;
}
