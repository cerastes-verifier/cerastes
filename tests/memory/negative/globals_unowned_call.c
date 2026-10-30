// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/verifast/globals.c
// Pins: 0041 item 8 and the port of VeriFast's globals: a helper whose contract names no global calls m, which needs owns(x): permission at the call
#include <cerastes.h>

static int x;

void m(void)
    CERASTES_REQUIRES(cerastes_owns(x) && x == 7)
    CERASTES_ENSURES(cerastes_owns(x) && x == 8)
{
  int y = x;
  x = y + 1;
}

void helper(void)
{
  m();
}

int main(void)
{
  int x0 = x;
  CERASTES_CHECK(x0 == 0);
  x = 7;
  m();
  int x1 = x;
  CERASTES_CHECK(x1 == 8);
  return x0 + x1 - 8;
}
