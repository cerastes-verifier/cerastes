// expect: rejected cerastes-unsupported_expression @ L16
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: main cannot be called (nothing may assume its result)
// Reported at the call (the rejection of a call of main is the encoder's).
#include <cerastes.h>

int main(void)
{
  return 0;
}

int again(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
{
  return x + main();
}
