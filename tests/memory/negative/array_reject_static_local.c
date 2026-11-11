// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2: a static array local is rejected (item 2); static arrays are e1e's storage (lifted by M4.5e1e)
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] static int a[3] = {1, 2, 3};
  return 0;
}
