// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 2, 0060 item 1 (flip B): ref_to_uninit on an array local is rejected; the text now says an array is not a pointer and names no session (P4222's markers are on pointers; an array parameter is a pointer parameter, record 0060 item 15)
// Pre-registered from record 0059 item 2
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] CERASTES_REF_TO_UNINIT int a[3];
  return 0;
}
