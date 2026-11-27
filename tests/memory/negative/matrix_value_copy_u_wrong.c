// expect: error check @ L27
// mode: c23, c++20
// twin: tests/memory/matrix_value_copy_u.c
// Pins: 0064 item 12 ("Tested so", the whole copy; question 353): after w = u the member c of w holds 8 (u.c), so a check that it holds 7 fails at the check; the right-value sibling is copy_cells in tests/memory/matrix_value_copy_u.c (w.c == 8); the self-copy's twin is matrix_value_self_copy_wrong.c
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): the one error at the wrong check; corrected once after the first probe on edg47/edg48 (program only, no expect line moves, the row's line is unchanged): the function returned 0 and EDG warned #550-D (the variable is set but never used, a check is not a use), so the return now reads the variable times 0; changed once after the second probe, before the third (program only, no expect line moves, and the rows' lines move by none: the destination is declared without the zero initializer, which the assignment w = u writes whole): the first form with a zero list for w took 55 to 66 s per mode at load 10 to 28
#include <cerastes.h>

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

int copy_wrong(void)
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
  CERASTES_CHECK(w.c == 7);  // <- here
  return w.a * 0;
}
