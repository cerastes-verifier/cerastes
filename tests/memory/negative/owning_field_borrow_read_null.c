// expect: error null-dereference @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_borrow_read.c
// Pins: 0055 item 3: the closure clause is p->data == NULL || owns_block(p->data), so the borrower may find the field NULL; reading *p->data without a test is a null dereference (the postcondition says nothing when the field is NULL, so the failed read flows into nothing that can fail)
// Pre-registered from record 0055 item 3: null-dereference at the read
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int first_or(struct buf *p, int dflt)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && (p->data == NULL || r == *p->data))
{
  return *p->data;  // <- here
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
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)malloc(sizeof(int));
  if (b->data == NULL) {
    buf_destroy(b);
    return -1;
  }
  *b->data = 9;
  int v = first_or(b, 5);
  buf_destroy(b);
  return v;
}
