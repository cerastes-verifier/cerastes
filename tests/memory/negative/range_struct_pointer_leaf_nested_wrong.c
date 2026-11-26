// expect: error check @ L43
// expect: error check @ L53
// expect: error check @ L60
// mode: c23, c++20
// twin: tests/memory/range_struct_pointer_leaf_nested.c
// Pins: 0064 item 3 ("A length quantifier under another quantifier"; audit round C2, finding 1): the same three functions with a claim that is false, one `check` each: count's n is only known to be positive (n > 1 is not provable), fill's p[0].k is 1 after the writes (not 2), second's p->n is not known to be 0. One reason each: the claim is not a fact of the contract and the writes. The rows are at their checks and no `lifetime` row stands beside them (no pointer is read through the range)
// Registered from il-analyst-report-11 (c) (w2f: `check(p[0].k == 2)` after the writes, measured: check; w3w: `check(p->n == 0)`, measured: check; k07's wrong twin on a load also drew `lifetime`, so this one claims about n instead, unmeasured), before the first run on the build with the fix; the rows' lines are inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct E2 {
  struct T *up;
  int k;
};

struct TT;

struct E {
  struct TT *up;
  int k;
};

struct TT {
  struct E v[2];
  int n;
};

void count(struct T **pp, int n)
    CERASTES_REQUIRES(n > 0 && cerastes_reads_range(pp, n))
    CERASTES_ENSURES(cerastes_reads_range(pp, n))
{
  (void)pp;
  (void)n;
  CERASTES_CHECK(n > 1);  // <- here
}

void fill(struct E2 *p, int n)
    CERASTES_REQUIRES(n > 1 && cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[0].k = 1;
  p[1].k = 2;
  CERASTES_CHECK(p[0].k == 2);  // <- here
}

int second(struct TT *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  CERASTES_CHECK(p->n == 0);  // <- here
  return p->v[1].k;
}
