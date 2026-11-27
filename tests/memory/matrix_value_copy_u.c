// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 ("Tested so", the whole copy; question 353): a whole-structure copy w = u of struct U { int a; int m[2][2]; int b[2]; int c; } writes every cell of w from the same cell of u (the non-array members a and c, the four cells of the matrix, the two of b): u holds a distinct value in every cell and every cell of w is read back by a check in code, the self-copy u = u is matrix_value_self_copy.c; negative/matrix_value_copy_u_wrong.c is the twin
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): verifies (time it first: two struct U locals in one function; split from the first draft's file, which held the self-copy too and took 54 s and 52 s at load 14); changed once after the second probe, before the third (program only, no expect line moves, and the rows' lines move by none: the destination is declared without the zero initializer, which the assignment w = u writes whole): the first form with a zero list for w took 55 to 66 s per mode at load 10 to 28
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

int copy_cells(void)
    CERASTES_ENSURES(r, r == 36)
{
  struct U u = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  struct U w;
  w = u;
  CERASTES_CHECK(w.a == 1);
  CERASTES_CHECK(w.m[0][0] == 2);
  CERASTES_CHECK(w.m[0][1] == 3);
  CERASTES_CHECK(w.m[1][0] == 4);
  CERASTES_CHECK(w.m[1][1] == 5);
  CERASTES_CHECK(w.b[0] == 6);
  CERASTES_CHECK(w.b[1] == 7);
  CERASTES_CHECK(w.c == 8);
  return w.a + w.m[0][0] + w.m[0][1] + w.m[1][0] + w.m[1][1] + w.b[0] + w.b[1] + w.c;
}
