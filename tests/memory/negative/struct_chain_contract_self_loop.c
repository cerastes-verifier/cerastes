// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/struct_chain_contract.c
// Pins: 0045 item 6: s.q == &s makes p->q->x the cell p->x that owns(*p) also holds: one cell twice (permission at the call; the static overlap rule cannot see it)
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

void inc(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x) && p->q->x < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x) && p->q->x == cerastes_old(p->q->x) + 1);

int use(void)
{
  struct L s = {0, 0};
  s.q = &s;
  inc(&s);  // <- here
  return s.x;
}
