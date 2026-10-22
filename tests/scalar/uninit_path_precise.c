// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 3: the flag is decided on the paths the conditions allow, not by a syntactic rule
#include <cerastes.h>

int guarded(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 0))
{
  int x;
  if (c != 0) {
    x = 1;
  }
  if (c != 0) {
    return x;
  }
  return 0;
}

int guarded_and(int c)
    CERASTES_ENSURES(r, r == (c > 5 ? c : 0))
{
  int x;
  if (c > 0) {
    x = c;
  }
  if (c > 0 && x > 5) {
    return x;
  }
  return 0;
}
