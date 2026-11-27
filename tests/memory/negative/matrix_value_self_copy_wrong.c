// expect: error check @ L25
// mode: c23, c++20
// twin: tests/memory/matrix_value_self_copy.c
// Pins: 0064 item 12 ("Tested so", the whole copy; question 353): after the self-copy u = u the cell u.m[1][0] still holds 4, so a check that it holds 3 fails at the check; the right-value sibling is self_copy in tests/memory/matrix_value_self_copy.c (u.m[1][0] == 4); split from matrix_value_copy_u_wrong.c for the 60 s line
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): the one error at the wrong check; corrected once after the first probe on edg47/edg48 (program only, no expect line moves, the row's line is unchanged): the function returned 0 and EDG warned #550-D (the variable is set but never used, a check is not a use), so the return now reads the variable times 0
#include <cerastes.h>

// the self-copy u = u written through a macro: clang's -Wself-assign, an error under the harness's -Werror, does not look inside a macro expansion, and the front end reads the expansion as the expression u = u
#define SELF_COPY(x) x = x

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

int self_copy_wrong(void)
{
  struct U u = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  SELF_COPY(u);
  CERASTES_CHECK(u.a == 1);
  CERASTES_CHECK(u.m[0][0] == 2);
  CERASTES_CHECK(u.m[0][1] == 3);
  CERASTES_CHECK(u.m[1][0] == 3);  // <- here
  CERASTES_CHECK(u.m[1][1] == 5);
  CERASTES_CHECK(u.b[0] == 6);
  CERASTES_CHECK(u.b[1] == 7);
  CERASTES_CHECK(u.c == 8);
  return 0;
}
