// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/not_invalidating_bodiless_second_tied.c
// Pins: 0058 item 2: the second parameter is not marked and its tie is missing, so touch2__wf fails at the ensures clause for q alone (N1: one message, the tie); the marker on p discharges only p's: the check at the call site is dropped, as in owning_field_bodiless_untied.c
// Pre-registered from record 0058 item 2 (N1): one permission error at the ensures clause; today's encoder reports two there, so this expectation holds only once N1 has landed
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch2(struct buf *p CERASTES_NOT_INVALIDATING, struct buf *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q));  // <- here

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
  struct buf *c = (struct buf *)calloc(1, sizeof(struct buf));
  if (c == NULL) {
    buf_destroy(b);
    return -1;
  }
  touch2(b, c);
  buf_destroy(b);
  buf_destroy(c);
  return 0;
}
