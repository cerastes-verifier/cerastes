// expect: error out-of-bounds @ L19
// expect: error out-of-bounds @ L31
// expect: error out-of-bounds @ L43
// expect: error uninitialized @ L56
// expect: error invariant-preserved @ L68
// expect: error out-of-bounds @ L88
// expect: error check @ L112
// expect: error termination @ L120
// mode: c23, c++20
// twin: tests/memory/matrix_loop_reach.c
// Pins: 0064 items 5 and 10 (Part E; audit round E2a, finding 3): the must-fail twin of matrix_loop_reach.c for the READS a loop makes of a local matrix, one error per function and each on its own index: an invariant that reads `m[0][i + 1]` (the second index may be 3: `out-of-bounds`), `m[i][0]` (the first index may be 2), `m[0][3 + i]` (always outside the row), a cell the matrix has not been given a value for (`uninitialized`), a value the loop breaks (`invariant-preserved`), a nested loop that writes `m[i][j]` for `j <= 3` (`out-of-bounds` at the write), a nested loop whose invariant does not keep the values (the `check` after it fails), and a measure that reads a cell that does not decrease (`termination`); the auditor's l06b, l06c, l06d, l06e, l06f, l14b, l14c and l15b
// Pre-registered from record 0064 items 5 and 10 (Part E; audit round E2a, finding 3), before any run: out-of-bounds at L19; out-of-bounds at L31; out-of-bounds at L43; uninitialized at L56; invariant-preserved at L68; out-of-bounds at L88; check at L112; termination at L120
#include <cerastes.h>

int second_index_high(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && m[0][i + 1] >= 0)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return m[0][0] * 0;
}

int first_index_high(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && m[i][0] >= 0)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return m[0][0] * 0;
}

int second_index_flat(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && m[0][3 + i] >= 0)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return m[0][0] * 0;
}

int unwritten_cell(void)
{
  int m[2][3];
  m[0][0] = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && m[0][1] >= 0)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return m[0][0] * 0;
}

int value_broken(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && m[1][2] == 6)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    m[1][2] = 7;
    i = i + 1;
  }
  return m[0][0] * 0;
}

int nested_write_high(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 3)
    CERASTES_DECREASES(4 - j)
    while (j <= 3) {
      m[i][j] = 0;  // <- here
      j = j + 1;
    }
    i = i + 1;
  }
  return m[0][0] * 0;
}

int nested_values_not_kept(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 3 && 0 <= i && i < 2)
    CERASTES_DECREASES(3 - j)
    while (j < 3) {
      m[i][j] = 0;
      j = j + 1;
    }
    i = i + 1;
  }
  CERASTES_CHECK(m[1][2] == 0);  // <- here
  return m[0][0] * 0;
}

int measure_does_not_decrease(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  CERASTES_INVARIANT(0 <= m[1][2] && m[1][2] <= 6)
  CERASTES_DECREASES(m[1][1])  // <- here
  while (m[1][2] > 0) {
    m[1][2] = m[1][2] - 1;
  }
  return m[1][2];
}
