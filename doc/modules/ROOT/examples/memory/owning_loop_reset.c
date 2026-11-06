// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning local the loop frees on its last iteration is set to NULL there; the invariant says so.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int last_frees(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 1;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n && (k > 0 || v == NULL))
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    if (k == 0) {
      free(v);
      v = NULL;
    }
  }
  return 0;
}
// end::source[]
