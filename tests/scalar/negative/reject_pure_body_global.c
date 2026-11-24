// expect: rejected cerastes-unsupported_expression @ L13
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3 and 0041 item 12: a global in the body of a read pure function is rejected by the check every spec routine gets first (a function it rejects is not read further: one diagnostic); measured on the frozen base build before the record's code
// Pre-registered from record 0062 (Part B), before the code: rejected. the one test of this list that the frozen base build gives today: unsupported_expression at the return, in both modes
#include <cerastes.h>

int gv;

CERASTES_PURE bool g(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return x >= gv;
}

int use(int y)
{
  return y;
}
