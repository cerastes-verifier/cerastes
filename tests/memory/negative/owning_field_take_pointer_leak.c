// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_take_pointer.c
// Pins: 0055 item 4 (N3): the block taken into the plain local t is never freed: the end-of-function row reports it at the closing line (t is used so that EDG's #177/#550 warnings do not fire)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), leak at the closing brace (the end-of-function row)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void take(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *t = p->data;
  p->data = NULL;
  if (t != NULL) {
    *t = 0;
  }
}  // <- here
