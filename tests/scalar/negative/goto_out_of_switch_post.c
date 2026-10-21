// expect: error postcondition @ L28
// mode: c23, c++20
// twin: tests/scalar/goto_out_of_switch.c
// Pins: 0015 item 2: the return after the label sees the goto path of case 2 (kind == 2, not 12)
// Only that path breaks the claimed postcondition; the other three paths satisfy it.
#include <cerastes.h>

int classify(int code)
    CERASTES_REQUIRES(0 <= code && code <= 10)
    CERASTES_ENSURES(r, (code == 0 && r == 0) || (code == 2 && r == 12) || (code == 1 && r == 11) || (code > 2 && r == 13))
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
