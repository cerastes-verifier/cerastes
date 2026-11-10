// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("Allocation views get none"): a structure from malloc, its fields then written, and one from calloc, whose owning field starts NULL; each is filled and consumed by the destructor, whose requires owns_block(p) takes the field's block with it
// Pre-registered from record 0055 item 3: verifies in both modes (the typed view of malloc carries no clause, so the field's block is the one the function stores; calloc's zeroed view holds no block in the field)
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
  CERASTES_CHECK(b->data == NULL);
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = 3;
  }
  buf_destroy(b);
  return 0;
}
