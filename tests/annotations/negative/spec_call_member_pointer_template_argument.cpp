// expect: frontend-error cerastes-spec_call_in_code @ L20
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_member.cpp
// Mutation of the twin: a pointer to the predicate member is a non-type template argument.
// Pins: plan R4b: spec_call_in_code for &S::ok as a template argument
#include <cerastes.h>

struct S {
  CERASTES_PREDICATE bool ok(int x) const { return x >= 0; }
};

int ident(const S& s, int x) CERASTES_REQUIRES(s.ok(x)) CERASTES_ENSURES(r, r == x)
{
  (void)s;
  return x;
}

template <bool (S::*P)(int) const> struct W { int v = 0; };

W<&S::ok> g_w;
