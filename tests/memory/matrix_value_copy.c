// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353, question 364's form): a whole-structure copy w = u of a structure with a matrix member (4 counted cells, under 7) writes each cell of w from the same cell of u: u holds a distinct value in every cell and each cell of w is read back by a check in code, which pins the copy's order of cells (a permutation of the matrix's cells is observable only here, a contract cannot read them); negative/matrix_value_copy_swapped.c is the twin
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run: verifies
#include <cerastes.h>

struct V {
  int m[2][2];
};

int copy(void)
    CERASTES_ENSURES(r, r == 10)
{
  struct V u = {{{1, 2}, {3, 4}}};
  struct V w = {{{0, 0}, {0, 0}}};
  w = u;
  CERASTES_CHECK(w.m[0][0] == 1);
  CERASTES_CHECK(w.m[0][1] == 2);
  CERASTES_CHECK(w.m[1][0] == 3);
  CERASTES_CHECK(w.m[1][1] == 4);
  return w.m[0][0] + w.m[0][1] + w.m[1][0] + w.m[1][1];
}
