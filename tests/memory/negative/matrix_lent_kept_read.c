// expect: error permission @ L23
// expect: error permission @ L30
// mode: c23, c++20
// twin: tests/memory/matrix_lent_back.c
// Pins: 0064 items 3, 5 and 8 (audit round E1, finding 4; the auditor's l04 and l06): after a callee kept what it was lent, reading the lent cell (m[1][1]) or a cell of the lent first row (m[0][1]) is `permission` at the read: the callee may have kept its object. The other cells of the matrix are not lent and read fine
// Pre-registered from record 0064 items 3, 5 and 8 (Part E), before any run: permission at the read of the lent cell, and at the read of the lent row's cell
#include <cerastes.h>

void keep_cell([[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
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
  return m[1][1];  // <- here
}

int rows(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  keep_rows(m);
  return m[0][1];  // <- here
}
