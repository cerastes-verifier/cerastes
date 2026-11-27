// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 5 and 8 (audit round E1, finding 4; the scope exit when a callee lends nothing for good): a cell &m[1][1], a row m[1] and the first row of m given to a callee whose contract takes the permission and gives it back are the caller's again at the scope exit: nothing may still be in use. The cells outside what was lent keep their values (m[0][0], m[0][2], m[1][1] of the third function), and the lent cell can be written after the call. Derived from the auditor's probes l01, l03 and l05; the twins are negative/matrix_lent_kept_lifetime.c and negative/matrix_lent_kept_read.c
// Pre-registered from record 0064 items 3, 5 and 8 (Part E), before any run: verifies
#include <cerastes.h>

void give_cell([[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*q))
{
}

void give_row([[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns_range(q, 3))
    CERASTES_ENSURES(cerastes_owns_range(q, 3))
{
}

void give_rows([[maybe_unused]] int a[][3])
    CERASTES_REQUIRES(cerastes_owns_range(a, 1))
    CERASTES_ENSURES(cerastes_owns_range(a, 1))
{
}

int cell(void)
    CERASTES_ENSURES(r, r == 10)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  give_cell(&m[1][1]);
  m[1][1] = 9;
  return m[1][1] + m[0][0];
}

int row(void)
    CERASTES_ENSURES(r, r == 3)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  give_row(m[1]);
  return m[0][2];
}

int rows(void)
    CERASTES_ENSURES(r, r == 5)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  give_rows(m);
  return m[1][1];
}
