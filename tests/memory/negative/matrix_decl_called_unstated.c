// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/matrix_decl_called.c
// Pins: 0064 item 13 (audit round E1, finding 4; the auditor's e_bodiless_owns_writes): a declaration that OWNS the range and states nothing about its cells may have written any of them, so the caller cannot rely on the value m[0][0] had before the call: the postcondition r == 1 fails (at the closing brace or the return: to confirm at the first run)
// Pre-registered from record 0064 item 13 (Part E), before any run: postcondition fails once
#include <cerastes.h>

void take(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n));

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  take(m, 2);
  return m[0][0];  // <- here
}
