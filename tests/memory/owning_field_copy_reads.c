// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: a borrower that only reads *p builds a structure value from its scalar leaves with the owning leaf NULL, which makes no owner (live(p->data) is written because a copy of the pointer leaf is a use of the pointee, today's lifetime check)
// Pre-registered from record 0055 item 6 (copies and values): verifies in both modes (the frozen base, by hand, verifies the same program: no row is involved)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int total(struct buf *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && (p->data == NULL || cerastes_live(p->data)) && p->n >= 0 && p->n < 1000)
    CERASTES_ENSURES(r, cerastes_reads(*p) && (p->data == NULL || cerastes_live(p->data)) && r == p->n + 1)
{
  struct buf t = { NULL, p->n };
  return t.n + 1;
}
