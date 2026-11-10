// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 7: a loop that only reads through a borrowed owning field, with no written block atom: the closure's requires site is an automatic loop place, so the invariant may read *p->data (S40 (g) g1_read.c has the clause written, 3.0 s)
// Pre-registered from record 0055 item 7 (loops): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int sum(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->data != NULL && 0 <= n && n <= 100 && *p->data == 3)
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->data != NULL && r == 3 * n)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 3 * i && *p->data == 3)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *p->data;
    i += 1;
  }
  return s;
}
