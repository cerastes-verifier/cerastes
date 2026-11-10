// expect: error leak @ L25
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/owning_field_vlibc_view.c
// Pins: 0055 items 3, 4: the field of the vlibc_malloc'd structure is given a block that vlibc_free(a) then drops: reported by the end-of-function row at the closing line
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), N6 (item 3, "Allocation views close nothing"):  leak at the closing brace of from_malloc
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
  a->data = (int *)vlibc_malloc(sizeof(int));
  vlibc_free(a);
  return 0;
}  // <- here

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
