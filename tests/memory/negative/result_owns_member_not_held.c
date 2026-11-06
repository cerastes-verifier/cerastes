// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/result_owns_given_member.c
// Pins: 0052 item 2: the callee holds only owns(p->x) but returns owns(*r) of &p->y: the cell is neither covered nor held, so permission at the return
// Pre-registered from record 0052 item 2: permission at the return statement
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *member(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x))
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  return &p->y;  // <- here
}
