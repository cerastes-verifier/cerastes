// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2: a thread_local array is rejected (item 2) (lifted by M8 (concurrency))
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] static thread_local int a[3] = {1, 2, 3};
  return 0;
}
