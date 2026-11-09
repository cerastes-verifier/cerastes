// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 and 10 and audit round 2 (Parts C and D), probe e9: owns(g) of an owning global under a guard (the deep atom, a block site, cannot be conditional on a non-pointer fact here)
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration (as the encoder prints it)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void f(int c) CERASTES_REQUIRES(c == 0 || cerastes_owns(g)) CERASTES_ENSURES(c == 0 || cerastes_owns(g)) { (void)c; }  // <- here

int main(void)
{
  g = NULL;
  f(1);
  return g == NULL;
}
