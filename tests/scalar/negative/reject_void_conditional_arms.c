// expect: rejected cerastes-unsupported_type @ L19
// mode: c23, c++20
// twin: tests/scalar/call_discarded_cond.c
// Pins: 0021 (P37): a ?: with void arms has no value and VIR has no void conditional; it is rejected, whatever the arms are
#include <cerastes.h>

void v(int x)
{
  (void)x;
}

void w(int y)
{
  (void)y;
}

int f(int c, int x, int y)
{
  c ? v(x) : w(y);
  return c;
}
