// expect: error check @ L11
// mode: c23, c++20
// twin: tests/memory/global_zero_init.c
// Pins: 0041 item 3: z is 0 at program start, so z == 1 fails
#include <cerastes.h>

static int z;

int main(void)
{
  CERASTES_CHECK(z == 1);
  return z;
}
