// expect: frontend-error undefined_identifier @ L6
// The same mistake when the contract names a parameter: the parameter does not exist yet.
// tag::bad[]
#include <cerastes.h>

CERASTES_REQUIRES(x > 0) int scale(int x); // <- error
// end::bad[]
