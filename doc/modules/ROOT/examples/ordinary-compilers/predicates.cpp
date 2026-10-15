// expect: golden
// expect: native-ok
// predicate and pure come first in the declaration; both are inline.
#include <cerastes.h>

// tag::placement[]
CERASTES_PREDICATE constexpr bool small(int x)
{
  return x < 10;
}

CERASTES_PURE int twice(int x)
{
  return x + x;
}
// end::placement[]
