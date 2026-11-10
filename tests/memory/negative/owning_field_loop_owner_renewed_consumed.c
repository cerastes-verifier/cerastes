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
// Pins: 0056 item 8 (question 201; not a false alarm): renew takes owns(**pp) and returns only owns(*pp), so its contract consumes the structure's cells: the call itself fails the first time it is made on a structure whose cells it does not return (S41 (0) run 7: renew(pv); renew(pv); with no loop reports the second call as permission, then lifetime, permission and leak); the callee's requires and the frees after the loop are reported. The expectations are the errors the encoder reports today, kept by the owner's answer 201
// Pre-registered from the final re-audit (r5/renew_pp2.c), observed on the session build after the last fix: the expect lines are the actual outcome; M4.5d2b2c keeps them (question 201) and states the cause (S41 (0) run 7)
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
