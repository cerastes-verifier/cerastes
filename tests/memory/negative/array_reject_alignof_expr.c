// expect: rejected cerastes-unsupported_type @ L13
// expect: frontend-warning std_alignof_with_expr_arg @ L13
// mode: c23
// c-only: _Alignof of an expression is a GNU extension EDG accepts in C23 (alignof in C++ is the _cxx pin)
// twin: tests/memory/array_sizeof_value.c
// Pins: 0059 item 11 (audit round 3, item 1): only sizeof of an array local is lifted (to N * sizeof(T)); the alignment of an array expression is rejected like the alignment of an array type (0024 item 6), EDG's #2471 listed
// Pre-registered from audit round 3 of record 0059: the tag is the one of reject_alignof_array.c
#include <cerastes.h>

int align(void)
{
  int a[3] = {1, 2, 3};
  return (int)_Alignof(a) + a[0];
}
