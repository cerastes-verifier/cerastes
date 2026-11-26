// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 3, the bullet "A length quantifier under another quantifier" (audit round C2, finding 1): a range whose element holds a pointer to a structure with an array member of structures, as the length facts of the pointee are stated under the range's own binder k and the inner quantifier over the member's elements takes a binder the slot expression does not mention (k1). count: reads_range over `struct T **` (the audit's k07); fill: owns_range over a structure with a `struct T *` member, its int member written and read (j13); second: the recursive type `struct TT { struct E v[2]; ... }`, `struct E { struct TT *up; ... }` under reads(*p), the int member of an element read (j11b). The twin negative/range_struct_pointer_leaf_nested_wrong.c fails one check in each function. Before the fix every one of the three was an exit 3 (Silicon: duplicate identifier `k`)
// Registered from audit-C2 probes k07, j13 and j11b (exit 3 on snapshot a044aff629b8) and il-analyst-report-11 (c) (w2p: p[0].k = 1; p[1].k = 2 and the check verified; w3p: `return p->v[1].k` verified, on the fixed build); the checks of count and second are trivial true facts added here, unmeasured; before the first run on the build with the fix
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
  CERASTES_CHECK(n >= 1);
}

void fill(struct E2 *p, int n)
    CERASTES_REQUIRES(n > 1 && cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[0].k = 1;
  p[1].k = 2;
  CERASTES_CHECK(p[0].k == 1 && p[1].k == 2);
}

int second(struct TT *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  int v = p->v[1].k;
  CERASTES_CHECK(v == p->v[1].k);
  return v;
}
