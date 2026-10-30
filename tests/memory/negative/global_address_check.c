// expect: error check @ L12
// mode: c23, c++20
// twin: tests/memory/global_address.c
// Pins: 0041 item 6: the addresses of two globals differ, so &g == &h cannot be checked
#include <cerastes.h>

static int g = 3;
static int h = 7;

int distinct(void)
{
  CERASTES_CHECK(&g == &h);
  return 0;
}

int main(void)
{
  return g - h;
}
