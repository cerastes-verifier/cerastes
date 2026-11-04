// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/struct_chain_contract.c
// Pins: 0045 item 3: + 2 against the body's + 1: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

void inc(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x) && p->q->x < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x) && p->q->x == cerastes_old(p->q->x) + 2)
{
  p->q->x += 1;
}  // <- here
