// expect: error check @ L15
// mode: c23, c++20
// twin: tests/memory/global_pointer_global.c
// Pins: 0041 item 6: gp == &h in main, so the write through gp is the write of h: h == 7 fails afterwards
#include <cerastes.h>

static int h = 7;
static int *gp = &h;

int main(void)
{
  if (gp) {
    *gp = 1;
  }
  CERASTES_CHECK(h == 7);
  return 0;
}
