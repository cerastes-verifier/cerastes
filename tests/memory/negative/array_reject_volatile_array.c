// expect: rejected cerastes-unsupported_type @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2: a volatile element type is rejected (item 2) (lifted by M8 (volatile))
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] volatile int a[3] = {1, 2, 3};
  return 0;
}
