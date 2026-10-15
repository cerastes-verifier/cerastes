// expect: frontend-error cerastes-unsupported_annotation_expression @ L9
// expect: native-ok
// twin: tests/annotations/contracts/template_fn.cpp
// Mutation of the twin: a requires-expression with a parameter of its own in a dependent contract: the printer has no node for it, so the back end must not pass it on silently (PENDING: the tag is not in diagnostics.def yet).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
T inc(T v) CERASTES_REQUIRES(requires (T q) { q + v; }) CERASTES_ENSURES(r, r > v)
{
  return v + 1;
}

int use(int x)
{
  return inc(x);
}
