// expect: rejected cerastes-unsupported_annotation @ L26
// expect: rejected cerastes-unsupported_annotation @ L33
// expect: rejected cerastes-unsupported_annotation @ L39
// expect: rejected cerastes-unsupported_expression @ L45
// expect: rejected cerastes-unsupported_annotation @ L58
// expect: rejected cerastes-unsupported_annotation @ L67
// expect: rejected cerastes-unsupported_annotation @ L74
// mode: c23, c++20
// twin: tests/memory/range_struct_owns.c
// Pins: 0064 item 1's rows that stay rejected for structures: a range whose base is not a pointer parameter (owns_range(&p[1], n); M4.5e2), an atom on one element (owns(p[1]), owns(p[1].x); M4.5e2), &p[e] in a contract (M4.5e2; the tag is that of range_reject_address_in_contract.c), a range atom in a loop invariant (M4.5e2), a range over a result (M4.5e2), and a range whose element structure has an owning member (M4.8). One row per function, at the clause; the tags are those of the scalar pins of the same rules (array_reject_element_atom.c, range_reject_base_result.c, range_reject_range_in_invariant.c), to confirm at the first run
// Pre-registered from record 0064 item 1 (Part C1), before any run: rejected
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void base_part(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(&p[1], n))  // <- here
{
  (void)p;
  (void)n;
}

void elem_atom(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(p[1]))  // <- here
{
  (void)p;
}

void leaf_atom(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(p[1].x))  // <- here
{
  (void)p;
}

int address_in_contract(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1 && &p[1] != p)  // <- here
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
  return 0;
}

int atom_in_invariant(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns_range(p, 1))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
  return 0;
}

struct P *over_result(int n)
    CERASTES_ENSURES(r, cerastes_owns_range(r, n))  // <- here
{
  (void)n;
  return NULL;
}

void owning_member(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))  // <- here
{
  (void)p;
  (void)n;
}
