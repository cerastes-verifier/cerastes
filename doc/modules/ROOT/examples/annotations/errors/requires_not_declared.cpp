// expect: frontend-error undefined_identifier @ L6
// A name that does not exist: annotations are type-checked like any C++ expression.
// tag::bad[]
#include <cerastes.h>

int scale(int x) CERASTES_REQUIRES(limit > 0); // <- error
// end::bad[]
