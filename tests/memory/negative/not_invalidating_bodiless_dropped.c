// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/not_invalidating_bodiless.c
// Pins: 0058 item 2: the same contract without the marker and without a tie is owning_field_bodiless_untied.c's: touch__wf fails once at the ensures clause (N1: the tie is asserted above the partner-guard assert, so one message); the call site's check is dropped from the twin, as there: after an untied call the field is unknown
// Pre-registered from record 0058 item 2 (N1): one permission error at the ensures clause; today's encoder reports two there (owning_field_bodiless_untied.c, before N1), so this expectation holds only once N1 has landed
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));  // <- here

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
  touch(b);
  buf_destroy(b);
  return 0;
}
