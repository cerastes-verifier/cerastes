// expect: error invalid-free @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_reads.c
// Pins: 0055 item 3 ("reads and live close nothing"): reads(*p) and reads(*p->data) give no token, so the borrower cannot free the block of the owning field
// Pre-registered from record 0055 item 3: invalid-free at the free call (as heap_free_borrowed.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int peek(struct buf *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->data != NULL && cerastes_reads(*p->data))
    CERASTES_ENSURES(r, cerastes_reads(*p) && cerastes_reads(*p->data) && r == *p->data)
{
  free(p->data);  // <- here
  return 0;
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
    CERASTES_ENSURES(r, r == 4 || r == -1)
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
  *b->data = 4;
  int v = peek(b);
  buf_destroy(b);
  return v;
}
