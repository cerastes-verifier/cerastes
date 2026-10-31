// expect: rejected cerastes-unsequenced_modification @ L13
// mode: c23, c++20
// twin: tests/memory/global_sequencing.c
// Pins: 0041 item 11 (0039 item 3, P63): g++ + g names g twice with a write between no sequence points; GCC's -Wsequence-point warns: rejected like the compilers (never lifted)
#include <cerastes.h>

static int g = 3;

int same(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  return g++ + g;
}
