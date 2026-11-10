// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 6 (limit 3): a loop that frees and replaces gp->data of an owning struct buf * global: the loop carries the block of gp->data as a closure site under the cells of gp's block, which are places for it; main ends holding gp (exempt)
// Pre-registered from record 0056 item 6 and S41 (a) (not yet run on the L-2 build): verifies in both modes; it is the former pin owning_field_loop_global_pointer_false_alarm.c (invalid-free x3 at the free, leak and permission x2 at the end on the base) with a reworded header
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING static struct buf *gp = NULL;

int main(void)
{
  gp = (struct buf *)calloc(1, sizeof(struct buf));
  if (gp == NULL) {
    return 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 5)
  CERASTES_DECREASES(5 - i)
  while (i < 5) {
    free(gp->data);
    gp->data = (int *)malloc(sizeof(int));
    if (gp->data != NULL) {
      *gp->data = i;
    }
    i += 1;
  }
  return 0;
}
