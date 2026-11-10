// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("reads and live close nothing"): reads(*p) gives the field's value only, so a borrower that reads *p->data writes reads(*p->data) as well (0045); the caller holds the block whole and lends a read share of both
// Pre-registered from record 0055 item 3: verifies in both modes
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
  return *p->data;
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
