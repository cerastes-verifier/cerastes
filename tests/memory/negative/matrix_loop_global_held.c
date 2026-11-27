// expect: error postcondition @ L25
// expect: error permission @ L36
// expect: error out-of-bounds @ L47
// expect: error permission @ L58
// expect: error check @ L77
// mode: c23, c++20
// twin: tests/memory/matrix_loop_global.c
// Pins: 0064 items 9 and 10 (Part E slice 3a; audit round E2a, finding 3): the must-fail twin of matrix_loop_global.c for a global matrix HELD by a function (`owns(g)` or `reads(g)`) and for a `static` local, one error per function: a loop that writes the global without an invariant for `g[0][0] == 1` (`postcondition`), a write under `reads(g)` (`permission`, at the write), an invariant that reads `g[0][i + 1]` (`out-of-bounds`: the second index may be 3), an invariant that reads `g[0][0]` in a function that holds nothing (`permission`), and a `static` local matrix whose loop stores by name and loses `c[0][0] == 1` (`check`); the auditor's l12, l12e, l12d and l13, and a second-index twin for a held global
// Pre-registered from record 0064 items 5, 9 and 10 (Part E; audit round E2a, finding 3), before any run: postcondition at L25; permission at L36; out-of-bounds at L47; permission at L58; check at L77
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int held_loses_the_value(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0][0] == 1)
    CERASTES_ENSURES(r, cerastes_owns(g) && g[0][0] == 1)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    g[i][1] = 0;
    i = i + 1;
  }
  return g[0][0] * 0;  // <- here
}

int write_under_reads(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    g[i][1] = 0;  // <- here
    i = i + 1;
  }
  return 0;
}

int invariant_reads_a_column_too_far(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[0][i + 1] <= 2147483647)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return 0;
}

int invariant_reads_with_no_atom(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && g[0][0] <= 2147483647)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return 0;
}

int static_local_loses_the_value(void)
{
  static int c[2][3] = {{1, 2, 3}, {4, 5, 6}};
  c[0][0] = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    c[i][1] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(c[0][0] == 1);  // <- here
  return c[1][2] * 0;
}
