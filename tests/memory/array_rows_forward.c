// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 13 (question 361, settled with the advisor on 2026-11-26): a row parameter forwarded whole to another function (g(a, n)) is accepted: the pointer and the count are the caller's, the atom is passed on and comes back, and the callee's write is known afterwards
// Pre-registered from record 0064 item 13 (Part E), before any run: verifies
#include <cerastes.h>

void g(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0 && n < 100)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0][0] == 1)
{
  (void)n;
  a[0][0] = 1;
}

void f(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0 && n < 100)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[0][0] == 1)
{
  g(a, n);
}
