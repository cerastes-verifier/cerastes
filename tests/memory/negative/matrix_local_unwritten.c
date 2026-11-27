// expect: error uninitialized @ L12
// mode: c23, c++20
// twin: tests/memory/matrix_local.c
// Pins: 0064 item 5: a cell of a local matrix that no statement wrote is read: uninitialized (the leaf's flag row), at the read, whatever else is written (the program has no postcondition, so nothing else is reported)
// Pre-registered from record 0064 item 5 (Part E), before any run: the one error
#include <cerastes.h>

int f(void)
{
  int m[2][3];
  m[0][0] = 1;
  return m[1][2];  // <- here
}
