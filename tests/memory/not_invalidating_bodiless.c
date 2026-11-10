// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2 ("Without one"): a bodiless marked borrower of a structure with an owning field writes no tie (owning_field_bodiless.c needs p->data == old(p->data)): the implicit ensures discharges 0054 item 2's tie in touch__wf and gives the caller, whose field is NULL, the same knowledge afterwards
// Pre-registered from record 0058 item 2: verifies in both modes (the written form, owning_field_bodiless.c, verifies today)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

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
  CERASTES_CHECK(b->data == NULL);
  buf_destroy(b);
  return 0;
}
