// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0055 item 3 ("Allocation views close nothing"): the same with vlibc's allocator: a structure with an owning field from vlibc_malloc and from vlibc_calloc, freed with vlibc_free and the field never written
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), N6 (item 3, "Allocation views close nothing"):  verifies (the edg-backend's v1.c a2/a4 and v2.c a4)
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int from_malloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *a = (struct buf *)vlibc_malloc(sizeof(struct buf));
  if (a == NULL) {
    return -1;
  }
  vlibc_free(a);
  return 0;
}

int from_calloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *a = (struct buf *)vlibc_calloc(1, sizeof(struct buf));
  if (a == NULL) {
    return -1;
  }
  vlibc_free(a);
  return 0;
}
