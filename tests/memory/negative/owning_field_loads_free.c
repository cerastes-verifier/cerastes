// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_loads.c
// Pins: 0055 item 3 (audit round 2 (d)): without the NULL store the closure of owns(**pp) has no block to take at the return (z_loads_twin.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), z_loads_twin.c: permission at the closing brace (the return row)
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
}  // <- here
