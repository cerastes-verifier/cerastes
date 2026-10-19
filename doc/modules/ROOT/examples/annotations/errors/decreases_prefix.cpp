// expect: frontend-error cerastes-contract_not_after_parameters @ L6
// A function's measure goes after the parameter list, like requires and ensures.
// tag::bad[]
#include <cerastes.h>

CERASTES_DECREASES(1) int count_down(int n); // <- error
// end::bad[]
