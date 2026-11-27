// expect: error division-by-zero @ L14
// expect: error division-by-zero @ L22
// expect: error division-by-zero @ L30
// expect: error division-by-zero @ L39
// expect: error division-by-zero @ L51
// mode: c23, c++20
// twin: tests/memory/array_rows_param_wf.c
// Pins: 0064 items 7 and 13 (as range_struct_wf_division.c, for the rows of a range; f__wf's ghosts, one per leaf path and slot, shared by no two): each function proves a[0][0] != 0 (or a[i][j] != 0) in its requires and divides by a different cell in its ensures, so the division is by zero in the ensures, once per function. other_col: a[0][1], another cell of the same row; other_row: a[1][0], the same column of another row; owned: under owns_range the body may change a[0][0], so the ensures' value is not the requires'; other_index: a[k][l] against a[i][j]; other_cell (question 367, registered before any run): a[0][1] != 0 in the requires and a division by a[1][1], a cell of another row AND column. The ruling's other function, the same contract under owns_range in both clauses, is `owned`. A shared ghost name would prove each of these
// Pre-registered from record 0064 items 7 and 13 (Part E), before any run: division-by-zero at the ensures line of each function
#include <cerastes.h>

int other_col(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 1 && a[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (r == 0 || r == 10 / a[0][1]))  // <- here
{
  (void)n;
  return a[0][0] * 0;
}

int other_row(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 1 && a[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (r == 0 || r == 10 / a[1][0]))  // <- here
{
  (void)n;
  return a[0][0] * 0;
}

int owned(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 1 && a[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && (r == 0 || r == 10 / a[0][0]))  // <- here
{
  (void)n;
  a[0][0] = 0;
  return 0;
}

int other_index(int a[][3], int n, int i, int j, int k, int l)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 0 <= i && i < n && 0 <= j && j < 3 && 0 <= k && k < n && 0 <= l && l < 3 && a[i][j] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (r == 0 || r == 10 / a[k][l]))  // <- here
{
  (void)n;
  (void)i;
  (void)j;
  (void)k;
  (void)l;
  return a[0][0] * 0;
}

int other_cell(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 1 && a[0][1] != 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (r == 0 || r == 10 / a[1][1]))  // <- here
{
  (void)n;
  return a[0][1] * 0;
}
