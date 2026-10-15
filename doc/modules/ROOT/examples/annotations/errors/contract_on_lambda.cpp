// expect: frontend-error cerastes-annotation_on_lambda @ L8
// A contract on a lambda: a lambda cannot carry one.
// tag::bad[]
#include <cerastes.h>

int use(int x)
{
  auto twice = [] CERASTES_REQUIRES(true) (int v) { return v + v; }; // <- error
  return twice(x);
}
// end::bad[]
