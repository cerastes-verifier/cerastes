// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 3 (re-audit S2): a non-owning pointer a shares the owner v's block at the loop's entry (a = v before the loop); the body re-points a at v, writes through it, frees the block and replaces it: a non-owning place that shares a block with an owner at entry stays off (the owner's carrier carries the block), so the replacement verifies; v is freed after the loop
// Pre-registered from record 0054 item 3 (re-audit S2): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int alias_shared(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 0;
  int *a = v;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n && v != NULL)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    a = v;
    *a = k;
    free(v);
    v = (int *)malloc(sizeof(int));
    if (v == NULL) {
      abort();
    }
    *v = 0;
  }
  free(v);
  return 0;
}
