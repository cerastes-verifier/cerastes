// expect: golden
// expect: native-ok
// ensures without and with a result name.
#include <cerastes.h>

// tag::plain[]
int next(int x) CERASTES_ENSURES(x < 1000)
{
  x = x + 1;
  return x;
}
// end::plain[]

// tag::binder[]
int abs_val(int x) CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x))
{
  return x < 0 ? -x : x;
}
// end::binder[]
