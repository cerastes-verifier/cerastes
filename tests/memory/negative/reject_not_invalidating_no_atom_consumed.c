// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_no_atom.c
// Pins: 0058 item 2: the same function with owns(*p) added to the requires only: the contract now takes the cell and returns nothing, which the marker forbids: unsupported_annotation at the parameter
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter
#include <cerastes.h>

int pass(int *p CERASTES_NOT_INVALIDATING, int n)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, r == n);

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  int v = pass(&x, 3);
  return v;
}
