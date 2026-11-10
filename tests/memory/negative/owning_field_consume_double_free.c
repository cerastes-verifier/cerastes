// expect: error invalid-free @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_consume.c
// Pins: 0055 item 3: the consumer has the field's block once; freeing p->data a second time is a double free
// Pre-registered from record 0055 item 3: invalid-free at the second free of p->data
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void dispose(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
  free(p->data);
  free(p->data);  // <- here
  free(p);
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)malloc(sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->n = 0;
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = 1;
  }
  dispose(b);
  return 0;
}
