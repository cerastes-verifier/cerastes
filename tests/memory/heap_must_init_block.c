// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 4 and 0042 item 4 (audit S4): an owns_block borrow on a must_init parameter: the callee writes the cell, the caller reads it after the call without having written it
// Pre-registered from record 0047 items 4, 8: verifies; re-registered after audit round 1 (record 0047 item 4): audit round 2 S4: BlockMode::Kept and an owns_block borrow on a must_init parameter had no pair
#include <cerastes.h>
#include <stdlib.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p) && *p == 0)
{
  *p = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 0;
  }
  init(p);
  int v = *p;
  free(p);
  return v;
}
