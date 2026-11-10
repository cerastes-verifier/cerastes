// expect: error invalid-free @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_guarded.c
// Pins: 0055 item 3 (audit round 2 (c)): on the reads arm the function holds no block of the field, so the free is not of an allocation it owns (p9_guard_twin.c)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), p9_guard_twin.c: invalid-free at the free on the reads arm
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void clear_if(struct buf *p, bool c)
    CERASTES_REQUIRES(c ? cerastes_owns(*p) : cerastes_reads(*p))
    CERASTES_ENSURES(c ? (cerastes_owns(*p) && p->data == NULL) : cerastes_reads(*p))
{
  if (c) {
    free(p->data);
    p->data = NULL;
  } else {
    free(p->data);  // <- here
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = 1;
  }
  clear_if(b, false);
  clear_if(b, true);
  free(b);
  return 0;
}
