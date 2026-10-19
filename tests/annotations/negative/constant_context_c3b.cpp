// expect: frontend-error cerastes-spec_call_in_code @ L18
// expect: native-ok
// twin: tests/annotations/spec_functions/constexpr_spec_functions.cpp
// Mutation of the twin: a static_assert in a body calls a constexpr spec function.
// Pins: plan R4a context C3b (il-inventory R4a): a spec function in a constant-evaluated context is ordinary code, rejected with spec_call_in_code
#include <cerastes.h>

CERASTES_PREDICATE constexpr bool q(int x) { return x >= 0; }
CERASTES_PURE constexpr int p(int x) { return x + 1; }

int bump(int x) CERASTES_REQUIRES(q(x) && x < 100) CERASTES_ENSURES(r, r == p(x))
{
  return x + 1;
}

int use(int x)
{
  static_assert(q(1), "q");
  return x;
}
