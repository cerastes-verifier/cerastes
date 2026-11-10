// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5, 0055 item 3 (audit round 2 (e)): the deep atom of an owning struct pointer global carries the closure of the structure's owning field: the function frees the field's block and sets it to NULL
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), o_gdeep.c: verifies in both modes
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING;

void reset(void)
    CERASTES_REQUIRES(cerastes_owns(gp))
    CERASTES_ENSURES(cerastes_owns(gp))
{
  if (gp != NULL) {
    free(gp->data);
    gp->data = NULL;
  }
}
