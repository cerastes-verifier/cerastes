// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("owning on a struct S * parameter"; 0040 item 3, 0055 item 8): cerastes::owning on a parameter stays a borrow (p == NULL || owns(*p) in and out) and its closure is the field's clause under the same guard; the body tests p, frees the field's block and sets it to NULL
// Pre-registered from record 0055 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void clear(struct buf *p CERASTES_OWNING)
{
  if (p != NULL) {
    free(p->data);
    p->data = NULL;
  }
}

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
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
  clear(b);
  buf_destroy(b);
  return 0;
}
