// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L18
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: the invariant names a static member of another class, written
// qualified (Cfg::i), and the for statement declares a variable named i.
// Pins: the check fails closed (CP8). The IL does not record whether a name was written
// qualified, and an unqualified i can find a member of an enclosing class, so every entity
// with the loop variable's name is reported.
#include <cerastes.h>

struct Cfg {
  static constexpr int i = 3;
};

int f(int n)
{
  int s = 0;
  CERASTES_INVARIANT(Cfg::i == 3) for (int i = 0; i < n; ++i) s += 1;
  return s;
}
