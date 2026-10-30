// expect: error termination @ L23
// mode: c23, c++20
// twin: tests/memory/noreturn_call_in_expression.c
// Pins: 0039 item 8 and 0021 item 4 (audit B2): without the decrement the recursive call does not decrease, and nr() in the same expression, which never returns, must not hide it: termination at the call
#include <cerastes.h>
#include <stddef.h>

int nr(void)
    CERASTES_ENSURES(r, false);

int down(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p >= 0 && *p < 100)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(res, cerastes_owns(*p))
{
  int a = 0;
  int b = 0;
  int *q = &a;
  int *s = &b;
  if (*p == 0) {
    return 0;
  }
  return (*q = nr()) + (*s = 1, down(p));
}
