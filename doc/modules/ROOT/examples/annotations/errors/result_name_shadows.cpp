// expect: frontend-error decl_hides_function_parameter @ L6
// A result name that is also a parameter name.
// tag::bad[]
#include <cerastes.h>

int next(int x) CERASTES_ENSURES(x, x > 0); // <- error
// end::bad[]
