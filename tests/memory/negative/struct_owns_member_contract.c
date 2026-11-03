// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/struct_owns_member.c
// Pins: 0043 items 9, 14: the contract reads p->y, which no atom frames: permission at the clause (a framing error in the contract)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x) && p->y > 0)  // <- here
{
  (void)p;
}
