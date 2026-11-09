// expect: error leak @ L29
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_load.c
// Pins: 0054 item 4: the loop overwrites s->q without freeing the block it held: the old block is held but no longer carried (the written site is keyed on the current value), so the iteration ends owning an allocation the loop does not carry
// Pre-registered from record 0054 item 4 and the category table ("for an iteration, the loop body's closing brace"): leak at the loop body's closing brace (BEST READING; the reassignment row of an owner does not apply to a member)
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int *q;
  int n;
};

void churn(struct S *s, int n)
    CERASTES_REQUIRES(cerastes_owns(*s) && cerastes_owns_block(s->q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*s) && cerastes_owns_block(s->q))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns_block(s->q))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    
    s->q = (int *)malloc(sizeof(int));
    if (s->q == NULL) {
      abort();
    }
    *s->q = i;
    i += 1;
  }  // <- here
}
