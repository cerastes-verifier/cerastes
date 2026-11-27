// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 (audit round C2, finding 3; the record names owns(p->v), this is the same atom on a global structure): owns(gt.v) and reads(gt.v) of the member array of a global structure are the atom on that array part alone: a callee that owns gt.v writes an element and the caller keeps gt.n (main's check after wv), a callee that reads gt.v returns an element of it, and a call takes the address of the other member gt.n beside owns(gt.v) (the two places are disjoint slots). The twin negative/struct_member_array_struct_gt_owns_permission.c writes gt.n or an element beside the wrong atom, and passes an element's address beside owns(gt.v)
// Registered from audit-C2 probe f02 (measured on snapshot a044aff629b8: wv, gq's call on &gt.n and its check gt.n == 99, and main's check of gt.n == 5 && gt.v[1].x == 9 verified; its designed-false checks gt.v[0].y == 2 in b and gt.v[0].x == 1 in main are dropped); rv with reads(gt.v) has NO probe: registered from record 0064 item 7, unmeasured
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

void wv(void)
    CERASTES_REQUIRES(cerastes_owns(gt.v))
    CERASTES_ENSURES(cerastes_owns(gt.v) && gt.v[1].x == 9)
{
  gt.v[1].x = 9;
}

int rv(void)
    CERASTES_REQUIRES(cerastes_reads(gt.v))
    CERASTES_ENSURES(r, cerastes_reads(gt.v) && r == gt.v[1].x)
{
  return gt.v[1].x;
}

void gq(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(gt.v))
    CERASTES_ENSURES(cerastes_owns(*q) && cerastes_owns(gt.v) && *q == 99)
{
  *q = 99;
}

int b(void)
    CERASTES_REQUIRES(cerastes_owns(gt))
    CERASTES_ENSURES(r, cerastes_owns(gt) && r == 0)
{
  gq(&gt.n);
  CERASTES_CHECK(gt.n == 99);
  return 0;
}

int main(void)
{
  wv();
  CERASTES_CHECK(gt.n == 5 && gt.v[1].x == 9);
  return gt.n - 5;
}
