// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (294; amends 0042, 0058 item 4; the pins struct_array_reject_markers.c's f_d09 (as a must_init filler that writes every element) and f_d13 (a ref_to_uninit local) and struct_array_reject_annotations.c's uninit_param): ref_to_uninit on a pointer to a structure with an array member (the caller's flags are kept, one quantifier per array leaf), a must_init filler that writes every element and the caller's read of an element afterwards, and a ref_to_uninit local passed on
// Pre-registered from record 0064 item 15 and the precedents init_ref_to_uninit.c, init_must_init.c and range_struct_marker.c, before any run: verifies (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void wr(struct S *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->a[0] = 7;
}

int keeps(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  wr(&s);
  int u = s.a[1];
  int w = s.n;
  return u == w;
}

void fill(struct S *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[0] == 1 && p->n == 4)
{
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->a[3] = 4;
  p->n = 4;
}

int use_fill(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s;
  fill(&s);
  int v = s.a[3];
  return s.a[0] + s.n + (v - v);
}

void g_d13(struct S *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*q))
{
  (void)q;
}

void f_d13(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  CERASTES_REF_TO_UNINIT struct S *q = p;
  g_d13(q);
}
