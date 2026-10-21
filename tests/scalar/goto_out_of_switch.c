// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2: a goto leaves a switch and skips the statements after it
// case 0 and case 2 jump past `kind += 10`; case 1 and the default reach it through break.
#include <cerastes.h>

int classify(int code)
    CERASTES_REQUIRES(0 <= code && code <= 10)
    CERASTES_ENSURES(r, (code == 0 && r == 0) || (code == 2 && r == 2) || (code == 1 && r == 11) || (code > 2 && r == 13))
{
  int kind = 0;
  switch (code) {
  case 0:
    goto done;
  case 1:
    kind = 1;
    break;
  case 2:
    kind = 2;
    goto done;
  default:
    kind = 3;
    break;
  }
  kind += 10;
done:
  return kind;
}
