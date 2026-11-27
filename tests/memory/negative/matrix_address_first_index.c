// expect: error out-of-bounds @ L12
// expect: error out-of-bounds @ L19
// mode: c23, c++20
// twin: tests/memory/matrix_address.c
// Pins: 0064 item 5 (audit round E1, finding 4; the auditor's a02 and a13): the address of the first cell of a row that does not exist, &m[2][0], and the decay of that row, m[2], are `out-of-bounds` on the FIRST index (one past the matrix is not a row to point into; the address &m[1][3], one past the end of row 1, is the allowed one, matrix_address.c)
// Pre-registered from record 0064 item 5 (Part E), before any run: out-of-bounds at each line
#include <cerastes.h>

int address(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  [[maybe_unused]] int *p = &m[2][0];  // <- here
  return 0;
}

int decay(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  [[maybe_unused]] int *p = m[2];  // <- here
  return 0;
}
