// expect: frontend-error expr_not_constant @ L16
// expect: native-ok
// twin: tests/annotations/spec_functions/constexpr_spec_functions.cpp
// Mutation of the twin: an array bound at namespace scope calls a constexpr spec function.
// Pins: plan R4a context C1 (il-inventory R4a): a spec function in a constant-evaluated context is ordinary code, refused by the front end (expr_not_constant: RAF_NO_CONSTANT_EVALUATION on predicate/pure, fork commit P)
#include <cerastes.h>

CERASTES_PREDICATE constexpr bool q(int x) { return x >= 0; }
CERASTES_PURE constexpr int p(int x) { return x + 1; }

int bump(int x) CERASTES_REQUIRES(q(x) && x < 100) CERASTES_ENSURES(r, r == p(x))
{
  return x + 1;
}

int g_array[p(3)];
