// expect: rejected cerastes-pure_function @ L8
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 items 1 and 3: more than the one return in a read pure body (an if with its own return) is pure_function, at the function (M4.5f1)
// Pre-registered from record 0062 (Part B), before the code: rejected. the tag and the line are question-160 candidates (as reject_pure_local_declaration.c)
#include <cerastes.h>

CERASTES_PURE int clamp0(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
{
  if (x < 0) {
    return 0;
  }
  return x;
}

int use(int z)
{
  return z;
}
