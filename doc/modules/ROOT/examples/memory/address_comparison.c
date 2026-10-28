// expect: verified
// expect: native-ok
// mode: c23, c++20
// The addresses of two different objects are different; a pointer equals the address it was given.
// tag::source[]
#include <cerastes.h>

int same_object(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a = 1;
  int b = 2;
  int *p = &a;
  CERASTES_CHECK(&a != &b);
  return p == &a && p != &b;
}
// end::source[]
