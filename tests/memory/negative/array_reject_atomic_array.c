// expect: rejected cerastes-unsupported_type @ L11
// mode: c23
// c-only: _Atomic is a keyword in C only (C++20 has std::atomic)
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2: an _Atomic element type is rejected (item 2) (lifted by M8 (_Atomic))
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] _Atomic int a[3] = {1, 2, 3};
  return 0;
}
