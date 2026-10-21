// expect: rejected cerastes-unsupported_statement @ L14
// c-only: C++ makes a jump past an initialized declaration a front-end error; C23 only warns (warning 546)
// twin: tests/scalar/goto_between_cases.c
// Pins: 0015 item 2: a goto never skips a declaration of the block that holds the label, in a switch body too
// y is declared in the switch body between the goto and the label, and is in scope at the label.
#include <cerastes.h>

int route(int code)
    CERASTES_REQUIRES(0 <= code && code <= 3)
{
  int kind = 0;
  switch (code) {
  case 0:
    goto shared;
  case 1:
    kind = 1;
    int y = 2;
    kind += y;
shared:
    kind += 5;
  }
  return kind;
}
