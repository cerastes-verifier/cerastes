// expect: error check @ L11
// c-only: the twin of a C-only test
// twin: tests/memory/global_linkage_c.c
// Pins: 0041 item 3: g starts as 5 in C: g == 6 fails
#include <cerastes.h>

int g = 5;

int main(void)
{
  CERASTES_CHECK(g == 6);
  return 0;
}
