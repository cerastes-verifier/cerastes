// expect: golden
// expect: native-ok
// requires on a declaration, and two requires on one declaration.
#include <cerastes.h>

// tag::single[]
int half(int x) CERASTES_REQUIRES(x >= 0);
// end::single[]

// tag::several[]
int clamp_pos(int x, int hi) CERASTES_REQUIRES(hi > 0) CERASTES_REQUIRES(x >= 0)
{
  return x < hi ? x : hi;
}
// end::several[]
