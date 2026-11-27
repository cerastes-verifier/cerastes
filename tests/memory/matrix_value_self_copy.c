// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 ("Tested so", the whole copy; question 353): the self-copy u = u of struct U { int a; int m[2][2]; int b[2]; int c; } keeps every cell (the non-array members a and c, the four cells of the matrix, the two of b): u holds a distinct value in every cell and every cell is read back by a check in code (split from matrix_value_copy_u.c for the 60 s line); negative/matrix_value_self_copy_wrong.c is the twin
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): verifies (time it first: one struct U local)
#include <cerastes.h>

// the self-copy u = u written through a macro: clang's -Wself-assign, an error under the harness's -Werror, does not look inside a macro expansion, and the front end reads the expansion as the expression u = u
#define SELF_COPY(x) x = x

struct U {
  int a;
  int m[2][2];
  int b[2];
  int c;
};

int self_copy(void)
    CERASTES_ENSURES(r, r == 36)
{
  struct U u = {1, {{2, 3}, {4, 5}}, {6, 7}, 8};
  SELF_COPY(u);
  CERASTES_CHECK(u.a == 1);
  CERASTES_CHECK(u.m[0][0] == 2);
  CERASTES_CHECK(u.m[0][1] == 3);
  CERASTES_CHECK(u.m[1][0] == 4);
  CERASTES_CHECK(u.m[1][1] == 5);
  CERASTES_CHECK(u.b[0] == 6);
  CERASTES_CHECK(u.b[1] == 7);
  CERASTES_CHECK(u.c == 8);
  return u.a + u.m[0][0] + u.m[0][1] + u.m[1][0] + u.m[1][1] + u.b[0] + u.b[1] + u.c;
}
