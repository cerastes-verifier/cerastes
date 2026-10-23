// expect: error precondition @ L17
// mode: c23, c++20
// The guard a >= 0 lets a == 0 reach the call, and pos requires a positive argument.
// tag::source[]
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int guarded(int a)
    CERASTES_ENSURES(r, r == (a > 1))
{
  return a >= 0 && pos(a) > 1;
}
// end::source[]
