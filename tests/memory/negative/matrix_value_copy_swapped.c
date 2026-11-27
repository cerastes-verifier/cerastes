// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/matrix_value_copy.c
// Pins: 0064 item 12 (question 353, question 364's form): after w = u the cell w.m[0][1] holds 2, so a check that it holds 3 (the value a transposed copy would write there) fails at the check; the other cells are checked right, so this is the one error
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run: the one error; corrected once after the first probe on edg47/edg48 (program only, no expect line moves, the row's line is unchanged): the function returned 0 and EDG warned #550-D (the variable is set but never used, a check is not a use), so the return now reads the variable times 0
#include <cerastes.h>

struct V {
  int m[2][2];
};

int copy(void)
{
  struct V u = {{{1, 2}, {3, 4}}};
  struct V w = {{{0, 0}, {0, 0}}};
  w = u;
  CERASTES_CHECK(w.m[0][0] == 1);
  CERASTES_CHECK(w.m[0][1] == 3);  // <- here
  CERASTES_CHECK(w.m[1][0] == 3);
  CERASTES_CHECK(w.m[1][1] == 4);
  return w.m[0][0] * 0;
}
