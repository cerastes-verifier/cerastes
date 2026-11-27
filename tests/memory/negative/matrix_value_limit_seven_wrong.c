// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/matrix_value_limit_seven_copy.c
// Pins: 0064 item 12 (question 353): the whole-structure copy at the limit (7 counted cells) writes every cell, the last one included: b.m[0][6] is 7 after b = a, so a check that it is 8 fails at the check (the error twin of the accepted side of the 7/8 pair; a rejected twin does not count for invariant 2)
// Pre-registered from record 0064 item 12 (Part E), before any run: the one error; corrected once after the first probe on edg47/edg48 (program only, no expect line moves, the row's line is unchanged): the function returned 0 and EDG warned #550-D (the variable is set but never used, a check is not a use), so the return now reads the variable times 0
#include <cerastes.h>

struct V7 {
  int m[1][7];
};

int copy7(void)
{
  struct V7 a = {{{1, 2, 3, 4, 5, 6, 7}}};
  struct V7 b = {{{0, 0, 0, 0, 0, 0, 0}}};
  b = a;
  CERASTES_CHECK(b.m[0][6] == 8);  // <- here
  return b.m[0][0] * 0;
}
