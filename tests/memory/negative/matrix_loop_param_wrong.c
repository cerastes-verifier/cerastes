// expect: error invariant-preserved @ L53
// expect: error permission @ L68
// expect: error permission @ L81
// expect: error permission @ L93
// expect: error permission @ L109
// expect: error postcondition @ L126
// expect: error postcondition @ L140
// expect: error postcondition @ L154
// mode: c23, c++20
// twin: tests/memory/matrix_loop_param.c
// Pins: 0064 items 5, 10 and 13 (Part E; audit round E2a, finding 3): the must-fail twin of matrix_loop_param.c for loops over an array-of-rows parameter's range, one error per function: an invariant the body breaks (`invariant-preserved`); an invariant that reads `a[0][3]` (a column the row does not have) or `a[i][0]` with `i <= n` (a row the range does not have) under `reads_range`, and one that reads a cell with no atom at all: each `permission` (the second index is tested against the row, the first against the range); a store under `reads_range` (`permission`, at the write); and a loop whose store, owning callee over the rows or owning callee over one row loses `a[0][2] == 7` (`postcondition`); the auditor's l07b, l07c, l07d, l07f, l08, l09, l09c and l09d
// Pre-registered from record 0064 items 5, 10 and 13 (Part E; audit round E2a, finding 3), before any run: invariant-preserved at L53; permission at L68; permission at L81; permission at L93; permission at L109; postcondition at L126; postcondition at L140; postcondition at L154
#include <cerastes.h>

void wrow(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0] = 0;
  (void)n;
}

int rrow(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == 0)
{
  (void)p;
  (void)n;
  return 0;
}

void wmat(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  a[0][0] = 0;
  (void)n;
}

int rmat(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == 0)
{
  (void)n;
  return a[0][0] * 0;
}

int invariant_broken(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][1] == 5)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || a[0][1] == 5))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a[i][1] = 5 + i;
    a[0][1] = 5 + i;
    i = i + 1;
  }
  return 0;
}

int invariant_reads_a_column_too_far([[maybe_unused]] int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 1 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && a[0][3] <= 2147483647)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
  return 0;
}

int invariant_reads_a_row_too_far([[maybe_unused]] int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 1 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && a[i][0] <= 2147483647)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
  return 0;
}

int invariant_reads_without_an_atom([[maybe_unused]] int a[][3], int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && a[0][0] <= 2147483647)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = i + 1;
  }
  return 0;
}

int writes_under_reads(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 1 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a[i][1] = 5;  // <- here
    i = i + 1;
  }
  return 0;
}

int store_loses_the_value(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100 && a[0][2] == 7)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][2] == 7)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a[i][1] = 5;
    i = i + 1;
  }
  return 0;  // <- here
}

int owning_callee_loses_the_value(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100 && a[0][2] == 7)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][2] == 7)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    wmat(a, n);
    i = i + 1;
  }
  return 0;  // <- here
}

int owning_row_callee_loses_the_value(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 1 <= n && n <= 100 && a[0][2] == 7)
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && a[0][2] == 7)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    wrow(a[i], 3);
    i = i + 1;
  }
  return 0;  // <- here
}
