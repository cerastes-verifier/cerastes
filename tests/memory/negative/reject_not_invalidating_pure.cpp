// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c++20
// twin: tests/memory/not_invalidating_cpp.cpp
// Pins: 0058 item 2 ("Never ignored"): not_invalidating on a parameter of a pure function is never silently ignored
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the declaration (as reject_marker_pure.cpp)
#include <cerastes.h>

CERASTES_PURE int pick(int *p CERASTES_NOT_INVALIDATING) { (void)p; return 0; }  // <- here

int main()
{
  return 0;
}
