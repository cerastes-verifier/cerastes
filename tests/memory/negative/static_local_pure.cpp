// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/static_local_unused.c
// Pins: 0041 item 12 (audit round 2, SF2): a static local in a pure function (its body is not verified before M4.5f1, and a pure function has no state)
#include <cerastes.h>

CERASTES_PURE int counter(int x) { static int c = 0; c++; return x + c; }

int main()
{
  return 0;
}
