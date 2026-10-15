// expect: frontend-error generic_lambda_not_scanned_again @ L12
// expect: frontend-error expansion_contains_no_packs @ L12
// expect: frontend-error no_matching_function @ L12
// expect: frontend-error bad_attribute_template_substitution @ L12
// expect: native-ok
// twin: tests/annotations/contracts/template_fn.cpp
// Mutation of the twin: a generic lambda with a parameter pack in a function template's contract: EDG refuses it when the template is instantiated (generic_lambda_not_scanned_again, reachable only in the cerastes configuration), and the failed substitution cascades into three more errors at the line; cerastes's own generic_lambda_in_annotation cannot fire because EDG errors first.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T inc(T v) CERASTES_REQUIRES([](auto a, auto... b) -> bool { return a > 0; }(v)) CERASTES_ENSURES(r, r > v)
{
  return v + 1;
}

int use(int x)
{
  return inc(x);
}
