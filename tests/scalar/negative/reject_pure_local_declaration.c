// expect: rejected cerastes-pure_function @ L8
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 items 1 and 3: a local declaration in a read pure body (the body is exactly one return, M4.5f1) is pure_function, at the function when only its requires makes it read
// Pre-registered from record 0062 (Part B), before the code: rejected. the tag and the line are question-160 candidates (the record names the rule and not the tag; the function's own line, as the record says for a function that only its contract makes read)
#include <cerastes.h>

CERASTES_PURE int dbl(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
{
  int y = x + x;
  return y;
}

int use(int z)
{
  return z;
}
