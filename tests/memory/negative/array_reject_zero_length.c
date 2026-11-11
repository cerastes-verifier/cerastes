// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1 and 2: a zero-length array is rejected (the bound is at least 1) (lifted by M8)
// Pre-registered from record 0059 item 1 and 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] int a[0];
  return 0;
}
