// expect: rejected cerastes-unsupported_expression @ L9
// twin: tests/memory/global_unnamed_namespace.cpp
// Pins: 0041 item 12 and 0054 item 10 and audit round 2 (Parts C and D), probe e11: an owning global read in a pure function: "a pure or predicate function that reads or writes a global" (global_pure_write.cpp already pins the write)
// Pre-registered from audit probe e11 as observed (the probe was C; here C++, where pure functions live): rejected unsupported_expression at the declaration
#include <cerastes.h>

CERASTES_OWNING static int *g = nullptr;

CERASTES_PURE int peek() CERASTES_REQUIRES(cerastes::reads(g)) { return g == nullptr; }  // <- here

int main()
{
  return 0;
}
