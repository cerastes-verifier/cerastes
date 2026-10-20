// expect: frontend-error attr_function_parameter_pack @ L11
// expect: native-ok
// twin: tests/annotations/spec_functions/forall_class_template.cpp
// Mutation of the twin: a member of a variadic class template passes its function parameter pack as the forall's free arguments (a pack named in a registered attribute stays rejected).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class... Ts>
struct Tup {
  int n;
  bool all_nonneg([[maybe_unused]] Ts... xs) const CERASTES_REQUIRES(cerastes::forall([](int i, Ts... ys) { return i >= 0; }, xs...))
  {
    return true;
  }
};

bool use(Tup<int, long>& t)
{
  return t.all_nonneg(1, 2L);
}
