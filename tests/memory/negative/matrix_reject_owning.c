// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/matrix_cells.c
// Pins: 0064 items 1 and 2 (M4.8; as array_reject_owning_array.c): owning on a matrix of pointers is rejected: an array of owning pointers needs a count
// Pre-registered from record 0064 items 1 and 2 (Part E), before any run: rejected (tag by array_reject_owning_array.c)
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] CERASTES_OWNING int *m[2][2];  // <- here
  return 0;
}
