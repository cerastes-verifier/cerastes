// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 3 (audit round 2 S2): two declarations with owning on one line (told apart by column) agree
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING int *p); void f(CERASTES_OWNING int *p) { if (p != NULL) *p = 1; }

int g(void)
{
  int x = 0;
  f(&x);
  return x;
}
