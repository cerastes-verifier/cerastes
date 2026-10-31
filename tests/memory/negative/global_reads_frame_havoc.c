// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/global_reads_frame.c
// Pins: 0041 item 9: a callee with owns(g) and no ensures about g's value may change it: the caller's check g == 3 after the call fails
#include <cerastes.h>

static int g = 3;

void clobber(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
}

int main(void)
{
  clobber();
  CERASTES_CHECK(g == 3);
  return g - g;
}
