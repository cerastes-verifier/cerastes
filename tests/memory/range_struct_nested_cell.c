// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 (the alias through a nested structure's leaf, and the first of the three pairings no fixture covers: a reads site against an owns): reads_range over struct S beside owns(*q); the caller passes the cell v[1].t.a, outside the range [0, 1). Two elements, three leaves each, kept minimal: the nested pairing programs were the slow ones (48 to 85 s) in the spike
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies (timed at the first run; the plan's rule 3)
#include <cerastes.h>
#include <stddef.h>

struct T {
  int a;
  int b;
};

struct S {
  int x;
  struct T t;
};

void nested(struct S *p, int n, int *q)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_owns(*q) && *q == 5)
{
  (void)p;
  (void)n;
  *q = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S v[2] = {{1, {2, 3}}, {4, {5, 6}}};
  nested(v, 1, &v[1].t.a);
  return v[1].t.a;
}
