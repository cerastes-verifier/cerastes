// expect: error invalid-free @ L22
// mode: c23, c++20
// twin: tests/memory/owning_field_get_bodiless.c
// Pins: 0055 item 3 (S2): the caller holds the field's block once and frees it twice
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), invalid-free at the second free
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *get(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

void user(void)
{
  struct buf *s = get();
  if (s != NULL) {
    free(s->data);
    free(s->data);  // <- here
  }
}
