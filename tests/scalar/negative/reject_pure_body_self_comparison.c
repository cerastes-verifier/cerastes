// expect: rejected cerastes-self_comparison @ L11
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3 and 0025 item 6 R5: the shape rules run in the body of a read pure function as in any expression: x == x is self_comparison (the function is read by its requires alone, so the line is the body's own)
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the ordinary form, measured on build/m4.5e1d/cerastes-base, is self_comparison at the comparison's line)
#include <cerastes.h>

CERASTES_PURE bool same(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return x == x;
}

int use(int y)
{
  return y;
}
