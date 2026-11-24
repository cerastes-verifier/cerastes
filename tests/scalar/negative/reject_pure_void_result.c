// expect: rejected cerastes-pure_returns_void @ L8
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 items 1 and 3 (M4.5f1): a pure function returning void is rejected by the check every pure function gets, which runs before anything else (pure_returns_void, at the function; measured on the frozen base build: unchanged by this session, so the record's pure_function does not apply to it)
// Pre-registered from record 0062 (Part B), before the code: rejected. Measured on build/m4.5e1d/cerastes-base: the test passes today in both modes
#include <cerastes.h>

CERASTES_PURE void nothing(int x)
    CERASTES_REQUIRES(0 <= x)
{
  (void)x;
}

int use(int y)
{
  return y;
}
