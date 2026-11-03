// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/memory/loop_heap_read_ptrglobal_measure.c
// Pins: 0044 item 12 (P69 c_dec_val): the measure *gp is constant: termination at the clause
#include <cerastes.h>

static int h = 10;
static int *gp = &h;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && gp == &h && h == 10)
  CERASTES_DECREASES(*gp)  // <- here
  while (i < 10) {
    
    i += 1;
  }
  return gp == &h ? 0 : 1;
}
