// expect: golden
// expect: native-ok
// The usual style: the contract on the declaration, none on the definition.
#include <cerastes.h>

// tag::style[]
int halve(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r <= x);

int halve(int x)
{
  return x / 2;
}
// end::style[]
