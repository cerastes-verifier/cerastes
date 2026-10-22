// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/scalar/uninit_goto_past_decl.c
// Pins: 0019 item 6: the label reads x, which the goto's path never declared or assigned
#include <cerastes.h>

int skip_decl(int a)
    CERASTES_REQUIRES(0 <= a && a <= 10)
    CERASTES_ENSURES(r, a > 5 || r == a)
{
  int total = 0;
  if (a > 5) {
    goto done;
  }
  int x;
  x = a;
  total = 1;
done:
  total = x;
  return total;
}
