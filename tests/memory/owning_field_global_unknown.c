// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 11 (S7): a struct global of unknown value (a tentative definition): main assigns the owning leaf before the call whose owns(g) closure needs it, so the clause g.data == NULL || owns_block(g.data) holds
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (the frozen base, clause by hand, verifies it in both modes: the global of a tentative definition is unknown in C++ too)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && g.data == NULL)
{
  free(g.data);
  g.data = NULL;
}

int main(void)
{
  g.data = NULL;
  f();
  return 0;
}
