// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 6: under NDEBUG assert expands to (void)0, so nothing is checked (C's meaning)
// The assertion would fail for every x; it is not there.
#define NDEBUG
#include <assert.h>
#include <cerastes.h>

int identity(int x)
    CERASTES_ENSURES(r, r == x)
{
  assert(x > 100);
  return x;
}
