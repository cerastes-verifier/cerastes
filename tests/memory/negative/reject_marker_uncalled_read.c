// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/init_uncalled.c
// Pins: 0042 items 4, 6 (audit round 2 S4): an uncalled declaration whose requires reads *p is rejected (a precondition that reads *p)
#include <cerastes.h>

void keep(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_REQUIRES(*p > 0);  // <- here

int main(void)
{
  return 0;
}
