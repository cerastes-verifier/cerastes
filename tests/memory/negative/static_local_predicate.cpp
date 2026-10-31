// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/static_local_unused.c
// Pins: 0041 item 12 (audit round 2, SF2): a static local in a predicate function
#include <cerastes.h>

CERASTES_PREDICATE bool ok(int x) { static int c = 0; c++; return x > c; }

int main()
{
  return 0;
}
