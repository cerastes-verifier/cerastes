// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/struct_chain_contract.c
// Pins: 0045 item 1: old inside a place's path, old(p)->q->x: M8 (listed)
#include <cerastes.h>
struct L {
  struct L *q;
  int x;
};

void f(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(p->q->x) && cerastes_old(p)->q->x == 1)  // <- here
{
  (void)p;
}
