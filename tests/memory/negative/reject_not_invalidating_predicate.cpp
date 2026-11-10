// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c++20
// twin: tests/memory/not_invalidating_cpp.cpp
// Pins: 0058 item 2 ("Never ignored"): not_invalidating on a parameter of a predicate (M4.5f1 decides markers in specification functions)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the declaration (as reject_marker_predicate.cpp)
#include <cerastes.h>

CERASTES_PREDICATE bool valid(int *p CERASTES_NOT_INVALIDATING) { return p != nullptr; }  // <- here

int main()
{
  return 0;
}
