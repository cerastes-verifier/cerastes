// expect: rejected cerastes-unsupported_declaration @ L14
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1: an array of structures is rejected (lifted by M4.5e1e)
// Pre-registered from record 0059 item 1
#include <cerastes.h>

struct S {
  int x;
};

int f(void)
{
  [[maybe_unused]] struct S a[2];
  return 0;
}
