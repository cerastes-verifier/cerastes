// expect: error unsequenced @ L29
// expect: error unsequenced @ L35
// mode: c23, c++20
// twin: tests/memory/array_rows_param_seq.c
// Pins: 0064 item 11 (the audit's finding 1 for ranges of structures, the same rows per leaf path of a row): beside_read, wr(m, 2) owns the cells of both rows and m[1][1] is read in the same expression; overlap, wr(m, 1) and wr(m, 2) own row 0 twice. With the sequencing rows made assert true nothing else rejects either
// Pre-registered from record 0064 item 11 (Part E), before any run: one unsequenced per function, at the line of its expression
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

int beside_read(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 5, 0}};
  return wr(m, 2) + m[1][1];  // <- here
}

int overlap(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 5, 0}};
  return wr(m, 1) + wr(m, 2);  // <- here
}
