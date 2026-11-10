// expect: error permission @ L38
// expect: error permission @ L38
// expect: error permission @ L38
// expect: error leak @ L40
// expect: error permission @ L40
// expect: error invalid-free @ L42
// expect: error invalid-free @ L42
// expect: error invalid-free @ L42
// expect: error invalid-free @ L43
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owning_local_free.c
// Pins: 0055 item 7 (a false alarm pinned; the program is correct): an address-taken owning local struct pointer renewed through pv, struct buf **pv = &v, by a callee in a loop: no closure is carried across a replacement of the owner, so the loop's carry of v loses its pointee's owning field and the callee's requires and the frees after the loop are reported. The expectations are the errors the encoder reports today
// Pre-registered from the final re-audit (r5/renew_pp2.c), observed on the session build after the last fix: the expect lines are the actual outcome, recorded as a pin
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void renew(struct buf **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || (cerastes_owns(**pp) && (*pp)->data == NULL)))
    CERASTES_ENSURES(cerastes_owns(*pp));

int f(void)
{
  struct buf *v CERASTES_OWNING = (struct buf *)calloc(1, sizeof(struct buf));
  if (v == NULL) {
    return 0;
  }
  struct buf **pv = &v;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(cerastes_owns(*pv) && pv == &v)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    renew(pv);
    i = i + 1;
  }
  if (v != NULL) {
    free(v->data);
    free(v);
  }
  return 0;
}
