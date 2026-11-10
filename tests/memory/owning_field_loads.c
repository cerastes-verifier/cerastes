// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 (audit round 2 (d)): an atom through a load, owns(**pp), closes with the field's block: the function frees it and sets it to NULL
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), z_loads.c: verifies in both modes
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void clear_through(struct buf **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp))
{
  free((*pp)->data);
  (*pp)->data = NULL;
}
