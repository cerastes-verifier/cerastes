// expect: frontend-error attr_result_name_void_function @ L6
// A result name on a function that returns void.
// tag::bad[]
#include <cerastes.h>

void clear(int x) CERASTES_ENSURES(r, x > 0); // <- error
// end::bad[]
