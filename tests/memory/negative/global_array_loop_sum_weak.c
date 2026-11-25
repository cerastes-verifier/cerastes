// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/memory/global_array_loops.c
// Pins: 0063 item 10: the sum loop with the invariant s <= i: the values 1, 2, 3 make s 3 at i == 2, so the invariant is not preserved (reported at the loop; the clauses stand on the while's line, as array_loop_entry_keep.c)
// Pre-registered from record 0063 item 10 (Part B), before any run: the one error
#include <cerastes.h>

static int g[3] = {1, 2, 3};

int main(void)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && 0 <= s && s <= i) CERASTES_DECREASES(3 - i) while (i < 3) {  // <- here
    s += g[i];
    i += 1;
  }
  return s - s;
}
