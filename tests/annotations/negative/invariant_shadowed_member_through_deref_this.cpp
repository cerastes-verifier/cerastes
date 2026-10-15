// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L15
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: the invariant names the class's own member as (*this).i, and the for
// statement declares a variable named i.
// Pins: (*this).i is treated as this->i (CP7); only this diagnostic is reported.
#include <cerastes.h>

struct S {
  int i = 0;

  int f(int n)
  {
    int s = 0;
    CERASTES_INVARIANT((*this).i >= 0) for (int i = 0; i < n; ++i) s += 1;
    return s;
  }
};
