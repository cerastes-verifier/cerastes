// expect: rejected cerastes-unsupported_annotation @ L7
// mode: c++20
// twin: tests/memory/init_cpp.cpp
// Pins: 0042 item 4 (audit round 2 S3): owning on a parameter of a pure function is never silently ignored
#include <cerastes.h>

CERASTES_PURE int pick(int *p CERASTES_OWNING) { return 0; }  // <- here

int main()
{
  return 0;
}
