// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/struct_chain_whole.c
// Pins: 0045 item 3: p->q->x == 1 against the body's 0: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

void zero(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*p->q))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(*p->q) && p->q->x == 1)
{
  p->q->x = 0;
}  // <- here
