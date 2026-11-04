// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/struct_chain_contract.c
// Pins: 0045 items 3, 4: p->q->x read in the contract with no atom on it: permission at the clause
#include <cerastes.h>
#include <stddef.h>
struct L {
  struct L *q;
  int x;
};

int get(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->q->x > 0)  // <- here
{
  (void)p;
  return 0;
}
