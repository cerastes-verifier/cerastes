// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/overlap_in_annotations.c
// Pins: 0025 item 6 R11: the same contract as the positive test, accepted by the front end, fails for a body that returns 0
#include <cerastes.h>

int always(int a)
    CERASTES_ENSURES(r, r == (a > 0 || a < 10))
{
  return 0;
}
