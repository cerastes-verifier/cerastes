// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L16
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: the invariant calls the class's own member function through the implicit
// this (a const member function: the object is an implicit cast of this); the for statement
// declares a variable with the function's name.
// Pins: only calls through an object other than this (v.size()) are exempt (CP8).
#include <cerastes.h>

struct S {
  int size() const { return 0; }

  int f(int n)
  {
    int s = 0;
    CERASTES_INVARIANT(size() >= 0) for (int size = 0; size < n; ++size) s += 1;
    return s;
  }
};
