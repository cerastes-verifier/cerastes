// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 4 (audit round 2 (e)): a block the function holds (owns_block(a)) is stored into the owning field of the global's structure: the value rule finds it held
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), o_gdeep_store.c with a held block in place of a live pointer: verifies in both modes
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING;

void put(int *a)
    CERASTES_REQUIRES(cerastes_owns(gp) && gp != NULL && gp->data == NULL && a != NULL && cerastes_owns_block(a))
    CERASTES_ENSURES(cerastes_owns(gp) && gp != NULL)
{
  gp->data = a;
}
