// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pre-registered from record 0054 item 4: a loop swaps two pointers that hold two allocations; the written owns_block sites in the invariant say that each pointer holds a whole allocation, and that they are two.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int swap_many(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns_block(p)) && (q == NULL || cerastes_owns_block(q)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int *t = p;
    p = q;
    q = t;
    i += 1;
  }
  free(p);
  free(q);
  return 0;
}
// end::source[]
