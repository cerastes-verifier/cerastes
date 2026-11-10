// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 5 (audit S1): a guarded atom on a struct global, c ? owns(g) : true: the closure and the leaf ghost follow the arm's guard; the function frees and nulls on the owning arm
// Pre-registered from audit round 2 on Parts B/C (probes r3/): ge_guard.c: verifies in both modes
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void f(bool c)
    CERASTES_REQUIRES(c ? cerastes_owns(g) : true)
    CERASTES_ENSURES(c ? cerastes_owns(g) : true)
{
  if (c) {
    free(g.data);
    g.data = NULL;
  }
}
