// expect: rejected cerastes-unsupported_type @ L8
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 1: a pointer to an array is rejected (lifted by M8)
// Pre-registered from record 0059 item 1
#include <cerastes.h>

int f(int (*p)[3])
{
  return 0;
}
