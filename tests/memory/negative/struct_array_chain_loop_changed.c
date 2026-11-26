// expect: error invariant-preserved @ L21
// mode: c23, c++20
// twin: tests/memory/struct_array_chain_loop.c
// Pins: 0064 item 15: the loop writes 1 into every element, so the written invariant's p->a[0] == 0 does not survive the first iteration: invariant-preserved, at the invariant clause
// Pre-registered from record 0064 item 15, before any run: invariant-preserved at the invariant clause (the postcondition holds by the invariant at exit)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void zero_fill(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[0] == 0)
{
  p->a[0] = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_owns(*p) && p->a[0] == 0)  // <- here
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = 1;
    i = i + 1;
  }
}
