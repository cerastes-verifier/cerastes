// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/global_array_loops.c
// Pins: 0063 item 10 (array_loop_entry_keep.c for an array global): a callee kept g[2] before a loop whose body reads g[0]: the loop begins without the permission to every element, a marked entry row reported at the loop (the clauses stand on the while's line)
// Pre-registered from record 0063 item 10 (Part B), before any run: the one error
#include <cerastes.h>

static int g[3] = {1, 2, 3};

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int main(void)
{
  int i = 0;
  int s = 0;
  dispose(&g[2]);
  CERASTES_INVARIANT(0 <= i && i <= 2) CERASTES_INVARIANT(s == i) CERASTES_DECREASES(2 - i) while (i < 2) {  // <- here
    s = s + g[0];
    i = i + 1;
  }
  return s;
}
