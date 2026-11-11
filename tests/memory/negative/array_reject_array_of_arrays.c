// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1: an array of arrays is rejected (lifted by M4.5e1e)
// Pre-registered from record 0059 item 1
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] int a[2][2];
  return 0;
}
