// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/result_owns_given_member.c
// Pins: 0052 item 2 (question 159, audit B4): a written atom never covers: owns(p->x) and owns(*r) in one ensures are separating, and the callee returns &p->x, so the separation row at the return fails (the callee that hands p->x over drops it from its ensures)
// Pre-registered from record 0052 item 2: permission at the return statement (the separation row)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *member(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x))
    CERASTES_ENSURES(r, cerastes_owns(p->x) && cerastes_owns(*r))
{
  return &p->x;  // <- here
}
