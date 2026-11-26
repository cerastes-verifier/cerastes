// expect: rejected cerastes-unsupported_annotation @ L31
// expect: rejected cerastes-unsupported_expression @ L41
// expect: rejected cerastes-unsupported_type @ L45
// expect: rejected cerastes-unsupported_type @ L50
// expect: rejected cerastes-unsupported_annotation @ L57
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_copy.c
// Pins: 0064 items 1 and 12: owns_range(p->v, n) with the array member of structures as a range's base (M4.5e2: a range's base is a pointer parameter); t.v[i] = mk(7), a structure value stored into an element of the member array (M4.8, question 333: the local array's form is array_struct_reject_more.c's); a structure holding more than 7 array-element leaves (v[4] of two-leaf structures: 8) as a by-value parameter and as a by-value result (M4.5f3; the limit is vir::kMaxByValueArrayElements, tag and rows as negative/struct_array_reject_limit.c's)
// Added row (L57, from the coordinator's message after the joined proof, before this file's first run): a contract read of an element of an array member of structures of a structure VALUE (`CERASTES_REQUIRES(t.v[1].y == 4)` on a by-value parameter) is not in record 0064 and stays rejected (the record will list it), `unsupported_annotation` at the contract's line
// Pre-registered from record 0064 items 1 and 12 (Part C2), before any run: rejected (the annotation tag by negative/struct_array_reject_annotations.c's owns_range row, the expression tag by negative/array_struct_reject_more.c's v[i] = f() row, the type tag by negative/struct_array_reject_limit.c; each to confirm at the joined run)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct Big {
  struct P v[4];
};

struct P mk(int a)
    CERASTES_ENSURES(r, r.x == a);

void range(struct T *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p->v, n))  // <- here
{
  (void)p;
  (void)n;
}

int into(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  t.v[i] = mk(7);  // <- here
  return t.n;
}

int param(struct Big b)  // <- here
{
  return b.v[0].x * 0;
}

struct Big result(void)  // <- here
{
  struct Big b = {{{0, 0}}};
  return b;
}

int by_value(struct T t)
    CERASTES_REQUIRES(t.v[1].y == 4)  // <- here
{
  return t.n;
}
