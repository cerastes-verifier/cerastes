// expect: rejected cerastes-unsupported_annotation @ L34
// expect: rejected cerastes-unsupported_annotation @ L37
// expect: rejected cerastes-unsupported_annotation @ L39
// expect: rejected cerastes-unsupported_annotation @ L44
// expect: rejected cerastes-unsupported_annotation @ L54
// expect: rejected cerastes-unsupported_annotation @ L62
// expect: rejected cerastes-unsupported_annotation @ L68
// expect: rejected cerastes-unsupported_annotation @ L75
// expect: rejected cerastes-unsupported_annotation @ L81
// expect: rejected cerastes-unsupported_annotation @ L87
// mode: c23, c++20
// twin: tests/memory/struct_array_owns_array.c
// Pins: 0063 items 1 and 7 (questions 292, 293, 294, 306): the annotation rows for a structure with an array member that stay rejected, ten lines, one each: an ensures atom with no requires partner (0052) on a result and on owns(p->a); ref_to_uninit and owning on a pointer to such a structure (the heap and the markers: M4.5e1e2); a written atom in a loop invariant on such a place; owns_range on a member base (M4.5e2) and cerastes_forall and a pure application with a member argument (M4.5e1e2; ref_to_uninit stands for must_init, which shares its code path in edg-backend's plan C4); a contract place through a heap read whose structure has an array member, an atom and an element read (question 306). Every tag and line is to confirm at the joined run (edg-backend's plan C4; if one line is not reported, split the file)
// Pre-registered from record 0063 item 1's table (Part C), before any run: rejected
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct R {
  struct S *q;
  int z;
};

CERASTES_PURE bool pos(int i, const int *p, int n)
{
  return !(0 <= i && i < n) || p[i] > 0;
}

struct S *mk(void)
    CERASTES_ENSURES(r, cerastes_owns(*r));  // <- here

void give(struct S *p)
    CERASTES_ENSURES(cerastes_owns(p->a));  // <- here

void uninit_param(struct S *p CERASTES_REF_TO_UNINIT)  // <- here
{
  (void)p;
}

void owning_param(CERASTES_OWNING struct S *p)  // <- here
{
  (void)p;
}

void loop_atom(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(*p))  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    i = i + 1;
  }
}

void forall_member(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_forall(pos, p->a, 4))  // <- here
{
  (void)p;
}

void range_member(struct S *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns_range(p->a, n))  // <- here
{
  (void)p;
  (void)n;
}

void apply_member(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && pos(1, p->a, 4))  // <- here
{
  (void)p;
}

int through_load_atom(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))  // <- here
{
  return p->z;
}

int through_load_read(struct R *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->a[0] > 0)  // <- here
{
  return p->z;
}

int call_mk(void)
{
  struct S *p = mk();
  return p == NULL;
}
