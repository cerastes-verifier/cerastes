// expect: error null-dereference @ L8
// mode: c23, c++20
// twin: tests/memory/owning_oneline_match.c
// Pins: 0040 item 3 (audit round 2 S2): the owning parameter may be null, so *p without a test applies * to null
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING int *p); void f(CERASTES_OWNING int *p) { *p = 1; }

int g(void)
{
  int x = 0;
  f(&x);
  return x;
}
