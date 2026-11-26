// expect: rejected cerastes-unsupported_annotation @ L23
// expect: frontend-warning set_but_not_used @ L19
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_init.c
// Pins: 0064 item 7 and question 346 (audit round C2, finding 4; fail closed, listed in the record): `cerastes_old(gt.v[1].x)` of an element of the member array of a GLOBAL structure in an `ensures` is rejected as `unsupported_annotation`, in both modes, where `cerastes_old(g[1].x)`, `cerastes_old(p->v[1].y)` and `cerastes_old(gt.n)` are accepted. Lifted when record 0064 lists it as supported. The by-value pass of the same element is the C-only pin negative/struct_member_array_struct_reject_global_element_value_c.c (C++ accepts that call)
// Registered from audit-C2 probe b11 (rejected at the line of its CERASTES_ENSURES, unsupported_annotation, exit 2, measured on snapshot a044aff629b8 in C); in c++20 the first run on j3e measured the same row (L26, the only error); the row's line is inferred from this layout. Split from the first registration (question 160 correction, advisor's ruling: the by-value row moved out)
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

void gf(struct P *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(gt))
    CERASTES_ENSURES(cerastes_owns(*q) && cerastes_owns(gt) && gt.v[1].x == cerastes_old(gt.v[1].x) && q->x == 99)  // <- here
{
  q->x = 99;
}
