// expect: frontend-error cerastes-spec_marker_on_special_member @ L11
// expect: native-ok
// twin: tests/annotations/spec_functions/spec_marker_first_declaration.cpp
// Mutation of the twin: the marker on a defaulted copy assignment (it has a side effect).
// Pins: 0006 item 10: a predicate or pure marker is refused on a special member function
#include <cerastes.h>


struct S {
  int a;
  CERASTES_PURE S& operator=(const S&) = default;
};
