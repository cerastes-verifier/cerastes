// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_consume.c
// Pins: 0055 item 4 (free(p) while a field holds a block): the consumer frees the structure but not the block its owning field holds; no row at free(p) (S40 (d)): the end-of-function row reports the block at the closing line
// Pre-registered from record 0055 item 4 (S40 (d), no dropped-block row): leak at the closing brace of dispose (the end-of-function row)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void dispose(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
  free(p);
}  // <- here

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
