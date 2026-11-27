// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 6 and 16: the initializers of a local matrix: a nested list reads one of its six values at a symbolic cell, a partial list zero-fills the rest of each row and the missing rows ({{1}, {4, 5}} is {{1, 0, 0}, {4, 5, 0}}), the empty list {} zero-fills every cell, and C's {0} (the only elision of braces that is accepted, an integer literal in the lone position; in C++ it is the front end's -Wmissing-braces shape, rejected by matrix_reject_init_cpp.cpp, so the function stands under #ifndef __cplusplus)
// Pre-registered from record 0064 items 6 and 16 (Part E), before any run: verifies
#include <cerastes.h>

int nested(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, 1 <= r && r <= 6)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return m[i][j];
}

int partial(void)
    CERASTES_ENSURES(r, r == 10)
{
  int m[2][3] = {{1}, {4, 5}};
  return m[0][0] + m[0][1] + m[0][2] + m[1][0] + m[1][1] + m[1][2];
}

int empty(void)
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {};
  return m[0][0] + m[1][2];
}

#ifndef __cplusplus
int zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {0};
  return m[0][0] + m[1][2];
}
#endif
