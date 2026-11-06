// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (0047 item 4): a block site on a formal with no requires partner on that formal: the callee holds the whole block through another formal (a == b) and returns it under b's name; the caller frees the block afterwards
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void rebadge(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns_block(a) && a == b)
    CERASTES_ENSURES(cerastes_owns_block(b))
{
  (void)a;
  (void)b;
}

int use_rebadge(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  rebadge(p, p);
  free(p);
  return 0;
}
