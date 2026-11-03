// expect: error termination @ L15
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrglobal_kept.c
// Pins: 0044 item 12 (P69 c_trap2_dec): the loop writes gp = &k, so *gp is unknown at the start of an iteration unless the invariant pins it: termination at the measure
#include <cerastes.h>

static int h = 10;
static int k = 10;
static int *gp = &h;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && h == 10 - i && k >= 0 && k <= 10 && *gp >= 0)
  CERASTES_DECREASES(*gp)  // <- here
  while (i < 10) {
    gp = &k;
    h -= 1;
    i += 1;
  }
  return gp == &k ? 0 : 1;
}
