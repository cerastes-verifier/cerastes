// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 7: the same loop with the invariant written on the leaf, s.data == NULL || owns_block(s.data) (check.cpp admits a block site rooted at a struct local's leaf in an invariant; today it is rejected, S40 (g) g2_named.c)
// Pre-registered from record 0055 item 7 (loops over a named leaf: new, no base form; S40 (g) g2_named*.c are today's rejection and false alarm): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int churn(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(s.data == NULL || cerastes_owns_block(s.data))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(s.data);
    s.data = (int *)malloc(sizeof(int));
    if (s.data == NULL) {
      return 1;
    }
    *s.data = i;
    i += 1;
  }
  free(s.data);
  return 0;
}
