// expect: frontend-error cerastes-spec_call_in_code @ L20
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_member.cpp
// Mutation of the twin: a pointer to the predicate member is taken in a body.
// Pins: plan R4b: spec_call_in_code for &S::ok in a function body
#include <cerastes.h>

struct S {
  CERASTES_PREDICATE bool ok(int x) const { return x >= 0; }
};

int ident(const S& s, int x) CERASTES_REQUIRES(s.ok(x)) CERASTES_ENSURES(r, r == x)
{
  (void)s;
  return x;
}

int use_pm(int x)
{
  [[maybe_unused]] auto pm = &S::ok;
  return x;
}
