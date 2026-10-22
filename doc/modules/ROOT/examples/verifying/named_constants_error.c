// expect: error shift @ L7
// mode: c23, c++20
// The same kind of program with a constant whose initializer shifts by 40: the error is at the constant.
// tag::source[]
#include <cerastes.h>

const int BIG = 1 << 40;

int scaled(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1)
    CERASTES_ENSURES(r, r == x * BIG)
{
  return x * BIG;
}
// end::source[]
