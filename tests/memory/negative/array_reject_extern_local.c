// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2: an extern array declared in a block is rejected (item 2) (lifted by M4.5e1e)
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] extern int a[3];
  return 0;
}
