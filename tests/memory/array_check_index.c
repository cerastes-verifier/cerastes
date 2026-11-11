// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4: a[i] read in CERASTES_CHECK is framed like a read in code: the index must be inside the array
// Pre-registered from the M4.5e1a sweep (report 5, s14): verifies
#include <cerastes.h>

int get(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
{
  int a[3] = {0};
  CERASTES_CHECK(a[i] == 0);
  return a[i];
}
