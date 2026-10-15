// expect: golden
// expect: native-ok
// The first contract on the annotations page; the page also shows its listing.
// tag::contract[]
#include <cerastes.h>

int abs_val(int x) CERASTES_REQUIRES(x > -1000000) CERASTES_ENSURES(r, r >= 0 && (r == x || r == -x))
{
  return x < 0 ? -x : x;
}
// end::contract[]
