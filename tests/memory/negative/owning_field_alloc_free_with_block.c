// expect: error leak @ L53
// mode: c23, c++20
// twin: tests/memory/owning_field_alloc.c
// position: the error is at the end-of-function leak row (0047 item 7) at the closing line, by record 0055 item 4 (S40 (d) decided against a row at free(p)): "from_calloc may return while it still owns an allocation that its ensures does not hand back"
// Pins: 0055 items 3, 4 ("A dropped block", "free(p) while a field holds a block"): the structure from calloc is freed while its owning field still holds the block malloc gave it
// Pre-registered from record 0055 items 3, 4: leak at the closing brace of from_calloc (the end-of-function row, record 0055 item 4)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int from_malloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)malloc(sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->n = 1;
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = 7;
  }
  buf_destroy(b);
  return 0;
}

int from_calloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = 3;
  }
  free(b);
  return 0;
}  // <- here
