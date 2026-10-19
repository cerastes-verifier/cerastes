// expect: error overflow @ L7
// An annotation means its C value, so its arithmetic is checked too.
// tag::source[]
#include <cerastes.h>

int larger(int a, int b)
    CERASTES_REQUIRES(a + b > 0)
{
  return a > b ? a : b;
}
// end::source[]
