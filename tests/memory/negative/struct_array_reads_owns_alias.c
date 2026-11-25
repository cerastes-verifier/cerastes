// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_reads_reads_alias.c
// Pins: 0063 items 7 and 8 (audit round 3 S2): the same call with the second atom owns(*q): a reader and a writer of one slot do not hold together, the call needs the permission twice (permission at the call)
// Pre-registered from audit round 3 S2 and record 0063 item 7 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void two(struct S *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_reads(*p) && cerastes_owns(*q))
{
  (void)p;
  (void)q;
}

int use(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  two(&s, &s.a[1]);  // <- here
  return s.n;
}
