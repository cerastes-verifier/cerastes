// expect: rejected subscript_out_of_range @ L18
// expect: rejected subscript_out_of_range @ L19
// expect: rejected subscript_out_of_range @ L20
// mode: c23, c++20
// twin: tests/memory/array_sum_two.c
// Pins: 0059 item 4 and the matrix (task 2a): a macro, an enumerator and a folded sum are the same constant as 3 (clang warns on each, in both modes)
// Pre-registered from record 0059 item 4 (as updated: the driver makes EDG's #175, subscript out of range, an error in verify mode, both modes)
#include <cerastes.h>

#define THREE 3

enum { K3 = 3 };

int sum_ends(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a[3] = {1, 2, 3};
  int m = a[THREE];
  int e = a[K3];
  int s = a[1 + 2];
  return a[0] + a[2] + m + e + s;
}
