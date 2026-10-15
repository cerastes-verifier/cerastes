// expect: frontend-error cerastes-contract_not_after_parameters @ L8
// A contract written before the declaration instead of after the parameter list.
// tag::bad[]
#include <cerastes.h>

int limit = 10;

CERASTES_REQUIRES(limit > 0) int scale(int x); // <- error
// end::bad[]
