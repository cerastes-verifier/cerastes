// expect: rejected cerastes-unsupported_type @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1 and 2: a const element type is rejected (item 2); const arrays are held below write after the initializer (lifted by M4.6)
// Pre-registered from record 0059 item 1 and 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] const int a[3] = {1, 2, 3};
  return 0;
}
