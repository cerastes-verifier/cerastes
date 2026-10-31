// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/global_loop.c
// Pins: 0041 item 10: a global the function holds is in every loop's invariant already: a written owns(g) in CERASTES_INVARIANT is rejected
#include <cerastes.h>

static int g = 3;

void spin(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && cerastes_owns(g))
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    i += 1;
  }
}

int main(void)
{
  return g - g;
}
