// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c23, c++20
// twin: tests/memory/init_uncalled.c
// Pins: 0042 item 4 (audit round 2 S4): a must_init declaration with no contract that nothing calls is rejected like a called one (no owns(*p) in its requires)
#include <cerastes.h>

void fill(int *p CERASTES_MUST_INIT);  // <- here

int main(void)
{
  return 0;
}
