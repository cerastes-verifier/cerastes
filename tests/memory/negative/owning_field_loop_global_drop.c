// expect: error leak @ L24
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global.c
// Pins: 0055 items 5, 7: the loop re-allocates g.data without freeing the block it holds: the reassignment row at the store (owning_global_loop_cycle_no_free.c)
// Pre-registered from record 0055 item 7 (loops over a global's leaf: new, no base form for the closure through owns(g) in a loop): leak at the store
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

static struct buf g = { NULL, 0 };

void churn(int n)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    g.data = (int *)malloc(sizeof(int));  // <- here
    if (g.data != NULL) {
      *g.data = i;
    }
    i += 1;
  }
}

int main(void)
{
  churn(3);
  return 0;
}
