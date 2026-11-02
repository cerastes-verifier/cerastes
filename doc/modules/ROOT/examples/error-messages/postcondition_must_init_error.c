// expect: error postcondition @ L14
// mode: c23, c++20
// must_init promises that *p has a value on return, and the path with c == 0 never writes it.
// tag::source[]
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT, int c)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (c != 0) {
    *p = 0;
  }
}
// end::source[]
