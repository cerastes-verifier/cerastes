// expect: rejected subscript_out_of_range @ L12
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 4: a negative constant index of a named array is rejected (EDG's #175 as an error): never a[SIZE_MAX]
// Pre-registered from record 0059 item 4 (as updated: the driver makes EDG's #175, subscript out of range, an error in verify mode, both modes)
#include <cerastes.h>

int sum_ends(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a[3] = {1, 2, 3};
  return a[-1] + a[2];
}
