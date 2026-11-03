// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrglobal_measure.c
// Pins: 0044 item 12 (P69 c_dec_unheld): the measure reads *gp, which f does not hold: permission at the clause
#include <cerastes.h>

static int h = 0;
static int *gp = &h;

int f(int n)
    CERASTES_REQUIRES(cerastes_owns(gp) && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == 0 && cerastes_owns(gp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i + *gp * 0)  // <- here
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
