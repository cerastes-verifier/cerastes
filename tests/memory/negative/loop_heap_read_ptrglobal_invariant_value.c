// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrglobal_invariant.c
// Pins: 0044 item 12 (P69 c_inv_val_pres): *gp == 0 in the invariant, the body increments h: invariant-preserved
#include <cerastes.h>

static int h = 0;
static int *gp = &h;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && gp == &h && *gp == 0)  // <- here
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    h += 1;
    i += 1;
  }
  return gp == &h ? 0 : 1;
}
