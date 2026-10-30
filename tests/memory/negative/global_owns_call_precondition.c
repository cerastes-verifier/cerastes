// expect: error precondition @ L19
// mode: c23, c++20
// twin: tests/memory/global_owns_call.c
// Pins: 0041 items 7, 8: main owns g (program_start) but g == 100 violates bump's g < 100: precondition at the call
#include <cerastes.h>

static int g = 3;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g == cerastes_old(g) + 1)
{
  g += 1;
}

int main(void)
{
  g = 100;
  bump();
  return 0;
}
