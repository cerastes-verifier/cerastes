// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_copy_reads.c
// Pins: 0055 item 6 (the value rule): struct buf t = *p; under reads(*p) would make a second owner of a block the function does not hold: "owning field `t.data` receives a pointer that is not the start of a block the function holds", at the copy; live(p->data) is in the contract so that today's lifetime check ("uses `p->data` after the object it points to may have ended", which the plain reads(*p) form reports first on the frozen base) lets the value rule be the only reason
// Pre-registered from record 0055 item 6 (copies and values): permission at the copy (new: no base form; the frozen base verifies this program, by hand)
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
  struct buf t = *p;  // <- here
  return t.n + 1;
}
