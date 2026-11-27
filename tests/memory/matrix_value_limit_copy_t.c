// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353, as amended on 2026-11-26): the count differs between a VALUE and a COPY: struct T { int m[2][3]; int a[2]; } copies (t = s) at 6 counted cells (the cells of its matrix member; the direct array member a goes by copy_arr and does not count) and is accepted, where the same structure by value counts 8 (a beside the matrix counts) and is rejected as a parameter (negative/matrix_value_limit_eight.c, param_t); the copy is read back at the last cell of the matrix and at the last cell of the direct array member
// Pre-registered from record 0064 item 12 (Part E), before any run (test-mutator-11, 2026-11-27: the first draft's copy_t, split out of matrix_value_limit_seven_copy.c for the 60 s line, with the read-back checks added so that the twin has a row): verifies (time it first); slimmed once after the second probe (59 s with four checks, 51 s in the other mode): two checks; changed once more after the third probe (52 s and 61 s with two checks): the destination is declared without the zero initializer, as in matrix_value_copy_u.c (program only, the row's line is unchanged)
#include <cerastes.h>

struct T {
  int m[2][3];
  int a[2];
};

int copy_t(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T s = {{{1, 2, 3}, {4, 5, 6}}, {7, 8}};
  struct T t;
  t = s;
  CERASTES_CHECK(t.m[1][2] == 6);
  CERASTES_CHECK(t.a[1] == 8);
  return t.m[1][2] * 0 + t.a[1] * 0;
}
