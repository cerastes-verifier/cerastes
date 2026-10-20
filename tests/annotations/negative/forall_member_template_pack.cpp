// expect: frontend-error attr_function_parameter_pack @ L12
// expect: native-ok
// twin: tests/annotations/spec_functions/forall_member_template.cpp
// Mutation of the twin: the member template is variadic and the forall passes the pack as its free arguments (a function parameter pack named in a registered attribute stays rejected).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class U>
struct Buf {
  U* data;
  template <class... Ts>
  bool all_ge([[maybe_unused]] Ts... los) CERASTES_REQUIRES(cerastes::forall([](int i, Ts... ls) { return i >= 0; }, los...))
  {
    return true;
  }
};

bool use(Buf<int>& b)
{
  return b.all_ge(1, 2L);
}
