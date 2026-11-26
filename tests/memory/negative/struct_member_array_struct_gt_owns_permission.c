// expect: error permission @ L34
// expect: error permission @ L41
// expect: error permission @ L49
// expect: error permission @ L63
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_gt_owns.c
// Pins: 0064 item 7 (audit round C2, finding 3): owns(gt.v) is the atom on the member array alone, so wn (a write of the other member gt.n) and wr (a write of an element under reads(gt.v), an atom that holds no write amount) fail at the write, and a (gq(&gt.v[0].y) beside owns(gt.v): the address is a slot of the array the callee's second atom owns, the same slot twice) and e (g1(&gt.v[1]) beside owns(gt): an element and the whole structure) fail at the call. One reason each: a permission the function does not hold
// Registered from audit-C2 probes f02 (measured on snapshot a044aff629b8: `permission` at wn's write and at the call of gq in a) and f03 (`g1(&gt.v[1])` beside owns(gt): `permission` at the call, in e); wr, a write under reads(gt.v), has no probe of exactly that form: the auditor's l03 measured `permission` for a write under reads(p->v) (its b) and under reads(gt) (its d), so the category is measured for the neighbours and the global member-array form is unmeasured; the rows' lines are inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

static struct T gt = {{{1, 2}, {3, 4}}, 5};

void gq(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(gt.v))
    CERASTES_ENSURES(cerastes_owns(*q) && cerastes_owns(gt.v) && *q == 99)
{
  *q = 99;
}

void wn(void)
    CERASTES_REQUIRES(cerastes_owns(gt.v))
    CERASTES_ENSURES(cerastes_owns(gt.v))
{
  gt.n = 9;
}

int a(void)
    CERASTES_REQUIRES(cerastes_owns(gt))
    CERASTES_ENSURES(r, cerastes_owns(gt) && r == 0)
{
  gq(&gt.v[0].y);
  return 0;
}

void wr(void)
    CERASTES_REQUIRES(cerastes_reads(gt.v))
    CERASTES_ENSURES(cerastes_reads(gt.v))
{
  gt.v[0].x = 9;
}

void g1(struct P *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(gt))
    CERASTES_ENSURES(cerastes_owns(*q) && cerastes_owns(gt) && q->x == 99)
{
  q->x = 99;
}

int e(void)
    CERASTES_REQUIRES(cerastes_owns(gt))
    CERASTES_ENSURES(r, cerastes_owns(gt) && r == 0)
{
  g1(&gt.v[1]);  // <- here
  return 0;
}

int main(void)
{
  return gt.n - 5;
}
