// expect: golden
// expect: native-ok
// Pins: requires on a definition (attribute between the parameter list and
// the body), including two requires on one function (both shown, in order).
#include <cerastes.h>

int clamp_pos(int x, int hi) CERASTES_REQUIRES(hi > 0) CERASTES_REQUIRES(x >= 0)
{
  return x < hi ? x : hi;
}
