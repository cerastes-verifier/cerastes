// expect: rejected subscript_out_of_range @ L12
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 4: a constant index past the end of a named array is rejected like clang's default -Warray-bounds (EDG's #175 as an error)
// Pre-registered from record 0059 item 4 (as updated: the driver makes EDG's #175, subscript out of range, an error in verify mode, both modes)
#include <cerastes.h>

int sum_ends(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a[3] = {1, 2, 3};
  return a[0] + a[3];
}
