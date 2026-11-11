// expect: rejected cerastes-unsupported_type @ L12
// expect: frontend-warning std_alignof_with_expr_arg @ L12
// mode: c++20
// twin: tests/memory/array_sizeof_value.c
// Pins: 0059 item 11 (audit round 3, item 1): alignof of an array local in pass:[C++] (EDG warns #2471: the standard operator takes a type) is rejected, as the alignment of an array type is
// Pre-registered from audit round 3 of record 0059
#include <cerastes.h>

int align(void)
{
  int a[3] = {1, 2, 3};
  return (int)alignof(a) + a[0];
}
