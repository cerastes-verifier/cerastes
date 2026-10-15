// expect: golden
// expect: native-ok
// A contract repeated on a redeclaration and on the definition, with renamed parameters.
#include <cerastes.h>

// tag::same[]
int bump(int x) CERASTES_REQUIRES(x > 0 && x < 100) CERASTES_ENSURES(r, r > x);
int bump(int y);
int bump(int z) CERASTES_REQUIRES(z > 0 && z < 100) CERASTES_ENSURES(r, r > z)
{
  return z + 1;
}
// end::same[]
