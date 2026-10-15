// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L17
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: the invariant names a member inherited from a base class (through the
// implicit this); the for statement's variable hides it.
// Pins: a member reached through a conversion of this to a base class is still a member of this (CP7).
#include <cerastes.h>

struct B {
  int i = 0;
};

struct D : B {
  int f(int n)
  {
    int s = 0;
    CERASTES_INVARIANT(i >= 0) for (int i = 0; i < n; ++i) s += 1;
    return s;
  }
};
