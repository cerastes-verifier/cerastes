// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 items 4, 8 and 0042 item 4 (audit S4): an owns_block borrow (the same atom in requires and ensures) on a ref_to_uninit parameter keeps the block and its flag as the caller had them (BlockMode::Kept): the callee writes then reads, and the caller's own write stays visible after the call
// Pre-registered from record 0047 items 4, 8: verifies; re-registered after audit round 1 (record 0047 item 4): audit round 2 S4: BlockMode::Kept and an owns_block borrow on a must_init parameter had no pair
#include <cerastes.h>
#include <stdlib.h>

int touch(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, cerastes_owns_block(p) && r == 5)
{
  *p = 5;
  return *p;
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 3;
  int t = touch(p);
  int v = *p;
  free(p);
  return v - v + t - t;
}
