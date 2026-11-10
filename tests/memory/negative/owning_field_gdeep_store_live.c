// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/owning_field_gdeep_store.c
// Pins: 0055 item 4 (audit round 2 (e)): the pointer stored is only live, not a block the function holds: "owning field `gp->data` receives a pointer that is not the start of a block the function holds" (o_gdeep_store.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), o_gdeep_store.c: permission at the store
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING;

void put(int *a)
    CERASTES_REQUIRES(cerastes_owns(gp) && gp != NULL && gp->data == NULL && cerastes_live(a))
    CERASTES_ENSURES(cerastes_owns(gp) && gp != NULL && cerastes_live(a))
{
  gp->data = a;  // <- here
  gp->data = NULL;
}
