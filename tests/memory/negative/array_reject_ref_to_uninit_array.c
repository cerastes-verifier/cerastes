// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2: ref_to_uninit on an array is rejected (item 2) (lifted by M4.5e1b)
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] CERASTES_REF_TO_UNINIT int a[3];
  return 0;
}
