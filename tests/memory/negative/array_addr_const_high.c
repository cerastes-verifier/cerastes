// expect: rejected subscript_out_of_range @ L12
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 4 and the matrix (task 2a): &a[4], two past the end, is rejected like clang's -Warray-bounds (it warns on &a[4] and &a[-1], not on &a[3])
// Pre-registered from record 0059 item 4 (as updated: the driver makes EDG's #175, subscript out of range, an error in verify mode, both modes)
#include <cerastes.h>

int sum_ends(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a[3] = {1, 2, 3};
  [[maybe_unused]] int *e = &a[4];
  return a[0] + a[2];
}
