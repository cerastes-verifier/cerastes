// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("Allocation views close nothing"): a structure with an owning field from malloc (whose typed view leaves the field without a value) and from calloc (whose field is NULL), freed with the field never written: neither view carries the closure clause, so freeing the structure leaves no block held
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), N6 (item 3, "Allocation views close nothing"):  verifies in both modes (the edg-backend's v1.c a1/a3 and v2.c a3)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int from_malloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *a = (struct buf *)malloc(sizeof(struct buf));
  if (a == NULL) {
    return -1;
  }
  free(a);
  return 0;
}

int from_calloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *a = (struct buf *)calloc(1, sizeof(struct buf));
  if (a == NULL) {
    return -1;
  }
  free(a);
  return 0;
}
