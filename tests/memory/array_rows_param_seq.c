// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part E; the sequencing rows of a call that holds a range of rows have nothing else behind them): a call that owns the rows [0, 1) beside a read of a cell of row 1, outside the callee's range, is accepted; a reads_range call beside a read of a cell it reads is accepted too. The twins in negative/array_rows_param_seq_unsequenced.c move one operand into the callee's range
// Pre-registered from record 0064 item 11 (Part E), before any run: verifies
#include <cerastes.h>

int wr(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][1] == 7 && r == 0)
{
  (void)n;
  a[0][1] = 7;
  return 0;
}

int rd(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == a[0][1])
{
  (void)n;
  return a[0][1];
}

int beside_outside(void)
    CERASTES_ENSURES(r, r == 5)
{
  int m[2][3] = {{0, 0, 0}, {0, 5, 0}};
  return wr(m, 1) + m[1][1];
}

int reads_beside(void)
    CERASTES_ENSURES(r, r == 5)
{
  int m[2][3] = {{0, 5, 0}, {0, 0, 0}};
  return rd(m, 2) + m[0][1] - 5;
}
