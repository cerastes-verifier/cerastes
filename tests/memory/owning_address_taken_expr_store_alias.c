// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 ("Stores through an alias") and audit round 2 (Parts C and D), probe g4: a store of NULL through an alias inside an expression while v holds nothing
// Pre-registered from derived from audit probe g4 (v starts NULL): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING int *v = NULL;
  int **q = &v;
  int r = (*q = NULL) == NULL;
  return r - 1;
}

int main(void)
{
  return f();
}
