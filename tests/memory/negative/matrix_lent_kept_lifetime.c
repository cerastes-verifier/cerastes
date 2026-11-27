// expect: error lifetime @ L29
// expect: error lifetime @ L36
// expect: error lifetime @ L43
// mode: c23, c++20
// twin: tests/memory/matrix_lent_back.c
// Pins: 0064 items 3, 5 and 8 (audit round E1, finding 4; the auditor's l01, l03 and l05): a callee whose contract takes a cell, a row or the first row of a matrix and does NOT give it back keeps it: at the scope exit the caller's matrix may still be in use (`lifetime`, at the `return` of each function: the scope exit's rows come before the jump to the end, as array_live_callee_keep.c and array_return_in_block_keep.c have it). One error per function; no postcondition or read follows the call in a way that would fail first
// Pre-registered from record 0064 items 3, 5 and 8 (Part E), before any run: three lifetime errors, one per function, at the return (corrected before any run from the closing brace, by the precedent array_live_callee_keep.c: its lifetime error is at the return line L16)
#include <cerastes.h>

void keep_cell([[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
{
}

void keep_row([[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns_range(q, 3))
{
}

void keep_rows([[maybe_unused]] int a[][3])
    CERASTES_REQUIRES(cerastes_owns_range(a, 1))
{
}

int cell(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  keep_cell(&m[1][1]);
  return m[0][0];  // <- here
}

int row(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  keep_row(m[1]);
  return m[0][0];  // <- here
}

int rows(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  keep_rows(m);
  return m[1][1];  // <- here
}
