// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 4 (a written site through a load, owns_block(s->q)) and item 2 ("Loops": the way out of heap_loop_replace_false_alarm.c): the loop replaces the block of a member each iteration; the written carrier is keyed on the current value of s->q, so the new block is carried and the old one was freed
// Pre-registered from record 0054 item 4: verifies in both modes
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
    free(s->q);
    s->q = (int *)malloc(sizeof(int));
    if (s->q == NULL) {
      abort();
    }
    *s->q = i;
    i += 1;
  }
}
