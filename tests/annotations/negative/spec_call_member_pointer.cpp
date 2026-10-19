// expect: frontend-error cerastes-spec_call_in_code @ L18
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_member.cpp
// Mutation of the twin: a pointer to the predicate member is taken (a ck_ptr_to_member constant naming the routine).
// Pins: plan R4b: spec_call_in_code for &S::ok, a pointer to a predicate member
#include <cerastes.h>

struct S {
  CERASTES_PREDICATE bool ok(int x) const { return x >= 0; }
};

int ident(const S& s, int x) CERASTES_REQUIRES(s.ok(x)) CERASTES_ENSURES(r, r == x)
{
  (void)s;
  return x;
}

bool (S::*g_pm)(int) const = &S::ok;
