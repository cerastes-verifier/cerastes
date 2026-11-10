// expect: error permission @ L27
// expect: error permission @ L27
// N1 of record 0058 leaves one message here; the line was: expect: error permission @ L27
// N1 of record 0058 leaves one message here; the line was: expect: error permission @ L27
// expect: error invalid-free @ L40
// expect: error invalid-free @ L40
// expect: error invalid-free @ L40
// expect: error lifetime @ L42
// expect: error leak @ L50
// expect: error permission @ L45
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global_pointer.c
// Pins: 0056 item 12 (question 210): a call whose contract takes an owning global's closure (`requires` and `ensures` owns(gp)) after the caller freed gp: the call's row for the closure reads gp->data before any row checks that the caller still holds gp's block, so Silicon reports an unmapped insufficient.permission and the driver exits 3 (also on the base). The fix orders the read after the block's rows; the errors are then all mapped. The contract's own four permission errors are the bodiless callee's (it returns the closure without taking it)
// Pre-registered from audit round 2 S3 (e4b.c: the mapped errors it reports today, e4c.c and e4d.c as the neighbours without the dangling gp) and question 210 (not yet run on the build with the fix): the nine mapped errors above plus a mapped permission at the call `renew_gp();` in place of the unmapped one, exit 1, never exit 3; question 160 applies to the call's category and line
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING static struct buf *gp = NULL;

void renew_gp(void)
    CERASTES_REQUIRES(cerastes_owns(gp))
    CERASTES_ENSURES(cerastes_owns(gp));

int main(void)
{
  gp = (struct buf *)calloc(1, sizeof(struct buf));
  if (gp == NULL) {
    return 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (gp != NULL && i == 0) {
      free(gp->data);
    }
    if (gp != NULL) {
      free(gp);
    }
    renew_gp();
    if (gp != NULL) {
      gp->n = i;
    }
    i = i + 1;
  }
  if (gp != NULL) {
    free(gp);
  }
  return 0;
}
