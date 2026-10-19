// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 13: an obligation inside a user macro's body is checked at each use (here none fails)
// The macro's body multiplies, so each use is checked separately.
#include <cerastes.h>

#define SCALE(v) ((v) * 1000)

int first(int a)
    CERASTES_REQUIRES(a >= 0 && a <= 2147483)
{
  return SCALE(a);
}

int second(int b)
    CERASTES_REQUIRES(b >= 0 && b <= 2147483)
{
  return SCALE(b);
}
