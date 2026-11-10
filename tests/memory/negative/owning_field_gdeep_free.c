// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/owning_field_gdeep.c
// Pins: 0055 item 3 (audit round 2 (e)): without the NULL store the global's deep atom has no field block at the return (o_gdeep_twin.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), o_gdeep_twin.c: permission at the closing brace (the return row)
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
  }
}  // <- here
