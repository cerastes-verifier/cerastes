// expect: error check @ L19
// mode: c23, c++20
// twin: tests/memory/matrix_value_limit_copy_t.c
// Pins: 0064 item 12 (question 353): after t = s the direct array member t.a holds 7 and 8 (it goes by copy_arr, not by cells), so a check that t.a[1] holds 7 fails at the check; the other check is right, and the right-value sibling is copy_t in tests/memory/matrix_value_limit_copy_t.c
// Pre-registered from record 0064 item 12 (Part E), before any run (test-mutator-11, 2026-11-27): the one error at the wrong check; slimmed with its positive (two checks), the row's line is 19; changed once more after the third probe (52 s and 61 s with two checks): the destination is declared without the zero initializer, as in matrix_value_copy_u.c (program only, the row's line is unchanged)
#include <cerastes.h>

struct T {
  int m[2][3];
  int a[2];
};

int copy_t_wrong(void)
{
  struct T s = {{{1, 2, 3}, {4, 5, 6}}, {7, 8}};
  struct T t;
  t = s;
  CERASTES_CHECK(t.m[1][2] == 6);
  CERASTES_CHECK(t.a[1] == 7);  // <- here
  return t.m[1][2] * 0 + t.a[1] * 0;
}
