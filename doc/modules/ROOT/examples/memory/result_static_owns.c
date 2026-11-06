// expect: verified
// expect: native-ok
// mode: c23, c++20
// With owns(*r) the caller may also read and write the static local through the result.
// tag::source[]
#include <cerastes.h>

int *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static int s = 6;
  return &s;
}

int main(void)
{
  int *w = where();
  *w = 3;
  int v = *w;
  CERASTES_CHECK(v == 3);
  return v - 3;
}
// end::source[]
