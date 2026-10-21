// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2: a forward goto between cases of one switch body (the label is a later statement of the body)
// case 0 jumps into the middle of case 2; the dispatch ran before the jump, so no case value is skipped.
#include <cerastes.h>

int route(int code)
    CERASTES_REQUIRES(0 <= code && code <= 3)
    CERASTES_ENSURES(r, (code == 0 && r == 5) || (code == 1 && r == 1) || (code == 2 && r == 7) || (code == 3 && r == 0))
{
  int kind = 0;
  switch (code) {
  case 0:
    goto shared;
  case 1:
    kind = 1;
    break;
  case 2:
    kind = 2;
shared:
    kind += 5;
    break;
  default:
    break;
  }
  return kind;
}
