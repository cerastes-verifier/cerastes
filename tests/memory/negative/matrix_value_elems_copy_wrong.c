// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/matrix_value_elems_copy.c
// Pins: 0064 item 12 (question 353, the second type of the "tested so" pair): after w = u the leaf w.v[0].a[1] holds 2, so a check that it holds 4 (the value of the next element's first cell, what a copy that took the elements' leaves in another order would leave there) fails at the check; the other cells are checked right, so this is the one error, and the right-value sibling is copy_cells in tests/memory/matrix_value_elems_copy.c
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): the one error at the wrong check; corrected once after the first probe on edg47/edg48 (program only, no expect line moves, the row's line is unchanged): the function returned 0 and EDG warned #550-D (the variable is set but never used, a check is not a use), so the return now reads the variable times 0
#include <cerastes.h>

struct W {
  int a[2];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int copy_wrong(void)
{
  struct T u = {{{{1, 2}, 3}, {{4, 5}, 6}}, 7};
  struct T w = {{{{0, 0}, 0}, {{0, 0}, 0}}, 0};
  w = u;
  CERASTES_CHECK(w.v[0].a[0] == 1);
  CERASTES_CHECK(w.v[0].a[1] == 4);  // <- here
  CERASTES_CHECK(w.v[0].n == 3);
  CERASTES_CHECK(w.v[1].a[0] == 4);
  CERASTES_CHECK(w.v[1].a[1] == 5);
  CERASTES_CHECK(w.v[1].n == 6);
  CERASTES_CHECK(w.k == 7);
  return w.k * 0;
}
