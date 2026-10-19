// expect: frontend-error cerastes-spec_marker_on_special_member @ L12
// expect: frontend-error cerastes-pure_returns_void @ L12
// expect: native-ok
// twin: tests/annotations/spec_functions/spec_marker_first_declaration.cpp
// Mutation of the twin: the marker on a destructor.
// Pins: 0006 item 10: a predicate or pure marker is refused on a special member function
#include <cerastes.h>


struct S {
  int a;
  CERASTES_PURE ~S() {}
};
