// expect: frontend-error attr_result_name_deduced_type @ L6
// A result name on a function whose return type is deduced.
// tag::bad[]
#include <cerastes.h>

auto next(int x) CERASTES_ENSURES(r, r > x) { return x + 1; } // <- error
// end::bad[]
