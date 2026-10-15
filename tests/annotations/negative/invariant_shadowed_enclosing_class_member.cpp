// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L17
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: the invariant names a static member of the enclosing class, which
// unqualified lookup finds from the nested class; the for statement's variable hides it.
// Pins: members of enclosing classes are checked too (CP8: a narrowing to the function's own
// class and its bases missed them).
#include <cerastes.h>

struct Outer {
  static int i;

  struct Inner {
    int f(int n)
    {
      int s = 0;
      CERASTES_INVARIANT(i <= n) for (int i = 0; i < n; ++i) s += 1;
      return s;
    }
  };
};

int Outer::i = 3;
