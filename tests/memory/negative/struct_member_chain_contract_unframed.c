// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/struct_member_chain_contract.c
// Pins: 0045 items 3, 4: the old rejected shape: the read p->q->x with no atom on it is a permission error
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

void f(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->x > 0)  // <- here
{
  (void)p;
}
