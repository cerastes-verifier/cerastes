// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c++20
// twin: tests/memory/init_cpp.cpp
// Pins: 0042 item 4 (audit S5): a marker on a parameter of a pure function is never silently ignored (M4.5f1 decides markers in specification functions)
#include <cerastes.h>

CERASTES_PURE int pick(int *p CERASTES_MUST_INIT) { return 0; }  // <- here

int main()
{
  return 0;
}
