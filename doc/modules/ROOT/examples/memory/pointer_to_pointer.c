// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer to a pointer: **pp writes the object, *pp = &y moves the pointer p.
// tag::source[]
#include <cerastes.h>

int retarget(void)
    CERASTES_ENSURES(r, r == 9)
{
  int x = 1;
  int y = 7;
  int *p = &x;
  int **pp = &p;
  **pp = 2;
  CERASTES_CHECK(x == 2);
  *pp = &y;
  CERASTES_CHECK(p == &y);
  return x + *p;
}
// end::source[]
