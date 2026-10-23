// expect: rejected cerastes-unsupported_expression @ L17
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6 and 0012 item 12 (M4.3a audit note 5): the condition of an assert is code and a contract-like expression; a comma operator with a call in it is rejected
#include <assert.h>
#include <cerastes.h>

int note(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int f(int x)
    CERASTES_REQUIRES(0 <= x)
{
  assert((note(x), x >= 0));
  return x;
}
