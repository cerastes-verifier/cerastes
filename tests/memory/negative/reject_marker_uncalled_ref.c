// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_uncalled.c
// Pins: 0042 item 4 (audit round 2 S4): a ref_to_uninit declaration whose requires has no owns(*p) atom, never called, is rejected
#include <cerastes.h>

void keep(int *p CERASTES_REF_TO_UNINIT) CERASTES_REQUIRES(p != 0);  // <- here

int main(void)
{
  return 0;
}
