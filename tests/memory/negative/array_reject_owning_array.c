// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1 and 2: owning on an array of pointers is rejected (items 1 and 2): an array of owning pointers needs a count (lifted by M4.8)
// Pre-registered from record 0059 item 1 and 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] CERASTES_OWNING int *a[2];
  return 0;
}
