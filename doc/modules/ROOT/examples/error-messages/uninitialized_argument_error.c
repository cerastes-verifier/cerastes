// expect: error uninitialized @ L14
// mode: c23, c++20
// set needs an object with a value, and x has none when it is passed.
// tag::source[]
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 1);

int use_set(void)
{
  int x;
  set(&x);
  return 0;
}
// end::source[]
