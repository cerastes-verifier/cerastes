// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L17
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: in a lambda that captures a copy of *this, the invariant calls the
// class's member function unqualified; the for statement declares a variable with its name.
// Pins: the object of the call (the address of the captured copy) counts as this (CP8b), so
// the call does not get the exemption of calls through another object (v.size()).
#include <cerastes.h>

struct S {
  int size() const { return 0; }

  int f(int n)
  {
    auto g = [*this, n] {
      int s = size();
      CERASTES_INVARIANT(size() >= 0) for (int size = 0; size < n; ++size) s += 1;
      return s;
    };
    return g();
  }
};
