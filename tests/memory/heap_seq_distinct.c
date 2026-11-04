// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 9 (0028 item 13, 0039 item 4): a free and a use of another block in one full expression are not in conflict: the liveness and token cells of two blocks held whole are different
// Pre-registered from record 0047 item 9: verifies
#include <cerastes.h>
#include <stdlib.h>

int use_live(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && r == 0);

int distinct(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  int *q = (int *)malloc(sizeof(int));
  if (p == NULL || q == NULL) {
    free(p);
    free(q);
    return -1;
  }
  int s = (free(p), 0) + use_live(q);
  free(q);
  return s;
}
