// expect: error permission @ L18
// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/range_struct_contract.c
// Pins: 0064 item 7 (as range_decreases_elem_unframed.c): with n >= 0 only, the measure p[0].x reads an element of the first slot that the range may not hold: permission at the decreases clause; and (the second row, for the structure-atom framing of record 0064 item 7, whose positive functions are atom_requires, atom_old and atom_decreases of range_struct_contract.c) a requires that reads p[1].x under owns(*p), a structure atom on slot 0 only: permission at the clause
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the decreases clause
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

int dec(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
    CERASTES_DECREASES(p[0].x)  // <- here
{
  (void)p;
  return n;
}

int elem1(struct P *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[1].x > 0)  // <- here
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p->x;
}
