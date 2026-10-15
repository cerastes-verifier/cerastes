// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L16
// expect: native-ok
// twin: tests/annotations/regressions/shadow_checks_not_other_names.cpp
// Mutation of the twin: the invariant names an enumerator of a scoped enumeration that
// `using enum` makes visible unqualified; the for statement's variable hides it.
// Pins: scoped enumerators are not exempt (CP8: C++20 using enum and using Mode::i make them
// findable unqualified).
#include <cerastes.h>

enum class Mode { i, j };

int f(int n)
{
  using enum Mode;
  int s = 0;
  CERASTES_INVARIANT(i != j) for (int i = 0; i < n; ++i) s += 1;
  return s;
}
