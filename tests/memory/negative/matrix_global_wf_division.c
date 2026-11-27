// expect: error division-by-zero @ L15
// expect: error division-by-zero @ L22
// expect: error division-by-zero @ L29
// expect: error division-by-zero @ L37
// mode: c23, c++20
// twin: tests/memory/matrix_global_wf.c
// Pins: 0064 items 9, 11 and 14 (as global_array_struct_wf_division.c for a global matrix; f__wf's ghosts, one per leaf path and slot): each function proves g[0][0] != 0 (or g[i][0] != 0) in its requires and divides by a different cell in its ensures. other_col: g[0][1]; other_row: g[1][0]; owned: under owns(g) the body may change g[0][0]; other_index: g[0][j] against g[i][0]
// Pre-registered from record 0064 items 9 and 11 (Part E), before any run: division-by-zero at the ensures line of each function
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int other_col(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[0][1]))  // <- here
{
  return g[0][0] * 0;
}

int other_row(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[1][0]))  // <- here
{
  return g[0][0] * 0;
}

int owned(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_owns(g) && (r == 0 || r == 10 / g[0][0]))  // <- here
{
  g[0][0] = 0;
  return 0;
}

int other_index(int i, int j)
    CERASTES_REQUIRES(cerastes_reads(g) && 0 <= i && i < 2 && 0 <= j && j < 3 && g[i][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[0][j]))  // <- here
{
  (void)j;
  return g[i][0] * 0;
}
