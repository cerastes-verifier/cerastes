// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: flip (a) of question 93, record 0043: the struct declaration that reject_struct.c pinned as rejected is accepted beside a scalar function
#include <cerastes.h>

struct point {
  int x;
  int y;
};

int ident(int a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}
