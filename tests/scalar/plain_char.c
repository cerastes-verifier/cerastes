// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 2 (P12): plain char is signed on this target, taken from EDG
// Target-dependent by design: on a target where plain char is unsigned the twin below
// would not fail the same way (the verifier reads the signedness from EDG, never assumes it).
#include <cerastes.h>

char to_char(int x)
    CERASTES_REQUIRES(x >= -128 && x <= 127)
{
  return x;
}
