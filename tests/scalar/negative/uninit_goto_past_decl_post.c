// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/uninit_goto_past_decl.c
// Pins: 0019 item 6: the goto is accepted, the label's code gives the wrong value
#include <cerastes.h>

int assign_at_label(int a)
    CERASTES_REQUIRES(0 <= a && a <= 10)
    CERASTES_ENSURES(r, r == 7)
{
  if (a > 5) {
    goto done;
  }
  int x;
  x = 3;
done:
  x = 8;
  return x;
}
