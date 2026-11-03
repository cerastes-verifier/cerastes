// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrglobal_invariant.c
// Pins: 0044 item 12 (P69 c_inv_unheld_taut2): f owns gp but not h, so *gp in its loop invariant is not held: permission at the clause
#include <cerastes.h>

static int h = 0;
static int *gp = &h;

int f(int n)
    CERASTES_REQUIRES(cerastes_owns(gp) && gp != 0 && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == 0 && cerastes_owns(gp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *gp == *gp)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return 0;
}

int main(void)
{
  int z = gp == 0;
  return z + f(3);
}
