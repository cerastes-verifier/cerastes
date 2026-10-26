// expect: verified
// expect: native-ok
// mode: c23, c++20
// A static assertion is checked as written, like the rest of the program: this one is true, and so is everything it computes.
// tag::source[]
#include <cerastes.h>

static_assert(sizeof(int) == 4, "int is 32 bits");
static_assert(65535u * 65537u == 4294967295u, "the product fits in unsigned");

int bump(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  static_assert((1 << 30) == 1073741824, "a shift that fits");
  return x + 1;
}
// end::source[]
