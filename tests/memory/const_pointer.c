// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 2, 3: a pointer to const; the conversion that only adds a qualifier is accepted; a const local is a cell
#include <cerastes.h>

int through_const(void)
    CERASTES_ENSURES(r, r == 12)
{
  int x = 1;
  int *p = &x;
  const int *q = p;
  const int k = 5;
  const int *pk = &k;
  *p = 7;
  return *q + *pk;
}
