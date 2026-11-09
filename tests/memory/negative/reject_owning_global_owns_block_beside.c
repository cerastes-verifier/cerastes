// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 and 10 and audit round 2 (Parts C and D), probe e10: owns_block(g) beside owns(g) of an owning global: the block site is already inside the deep atom ("beside", 0047 item 4)
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration (as the encoder prints it)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void f(void) CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns_block(g)) CERASTES_ENSURES(cerastes_owns(g)) { }  // <- here

int main(void)
{
  return g == NULL;
}
