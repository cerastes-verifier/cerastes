// expect: frontend-error wrong_entity_for_attribute @ L7
// A contract on something that is not a function.
// tag::bad[]
#include <cerastes.h>

int limit = 10;
int counter CERASTES_REQUIRES(limit > 0) = 1; // <- error
// end::bad[]
