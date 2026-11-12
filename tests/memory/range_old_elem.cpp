// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0060 item 5: cerastes::old(p[j]) under cerastes::owns_range, the C++ spelling (a reference bound to the subscript, P79 (d))
// Pre-registered from record 0060 item 5, before the code: verifies
#include <cerastes.h>

void set(int *p, int n, int i, int j, int v)
    CERASTES_REQUIRES(cerastes::owns_range(p, n) && 0 <= i && i < n && 0 <= j && j < n && j != i)
    CERASTES_ENSURES(cerastes::owns_range(p, n) && p[i] == v && p[j] == cerastes::old(p[j]))
{
  (void)n;
  (void)j;
  p[i] = v;
}
