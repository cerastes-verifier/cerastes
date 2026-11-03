// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrglobal_invariant.c
// Pins: 0044 item 12 (P69 c_trap2): the loop repoints gp to k and writes k, the invariant *gp == h leaves the pointer free: invariant-preserved
#include <cerastes.h>

static int h = 0;
static int k = 0;
static int *gp = &h;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && *gp == h)  // <- here
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    gp = &k;
    k = 3;
    i += 1;
  }
  return gp == &k ? 0 : 1;
}
