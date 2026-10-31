// expect: rejected cerastes-unsupported_expression @ L9
// mode: c++20
// twin: tests/memory/global_unnamed_namespace.cpp
// Pins: 0041 items 7 and 12 (audit round 2, SF2): a pure function that writes a global is rejected (a pure body has no effects; pure bodies are verified by M4.5f1)
#include <cerastes.h>

static int g = 0;

CERASTES_PURE int bump(int x) { g = x; return x; }

int main()
{
  int v = g;
  return v;
}
