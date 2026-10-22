// expect: error postcondition @ L15
// mode: c23, c++20
// twin: doc/modules/ROOT/examples/verifying/assert.c
// Pins: 0015 item 6: the exact ensures of the doc example notices a wrong result
// The function returns sum / 3, so the result is not the average the ensures pins.
#include <assert.h>
#include <cerastes.h>

int average(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 100)
    CERASTES_ENSURES(r, r == (a + b) / 2)
{
  int sum = a + b;
  assert(sum <= 200);
  return sum / 3;
}
