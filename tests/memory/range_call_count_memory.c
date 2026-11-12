// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 8 (audit S4): a count argument that reads memory (*q, owned by the caller) is bound to a local before the call's rows, so no marked row's bound reads the heap: the call verifies
// Pre-registered from record 0060 item 8 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void zero_first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

int use(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 2)
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 0)
{
  int a[4] = {1, 2, 3, 4};
  zero_first(a, *q);
  return a[0];
}
