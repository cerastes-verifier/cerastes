// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/global_loop_pointer_store.c
// Pins: 0041 item 10: without g == 5 in the invariant the loop's store through p may have changed g: the postcondition fails at the closing brace
#include <cerastes.h>

static int g = 5;

void fill(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g) && g == 5)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(g) && g == 5)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    *p = i;
    i += 1;
  }
}

int main(void)
{
  return g - g;
}
