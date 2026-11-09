// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_global_init_cleanup.c
// Pins: 0054 item 5 and 10 and audit round 2 (Parts C and D), probe e13: owns(g) of an owning global in a ?: arm
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration (as the encoder prints it)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void f(int c) CERASTES_REQUIRES(c ? cerastes_owns(g) : cerastes_reads(g)) CERASTES_ENSURES(c ? cerastes_owns(g) : cerastes_reads(g)) { (void)c; }  // <- here

int main(void)
{
  g = NULL;
  f(1);
  return g == NULL;
}
