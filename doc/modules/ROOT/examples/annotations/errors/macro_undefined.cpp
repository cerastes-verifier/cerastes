// expect: frontend-error cerastes-cerastes_macro_changed @ L6
// The front end defines the macro that switches the annotations on: it cannot be undefined.
// tag::bad[]
#include <cerastes.h>

#undef __CERASTES__ // <- error
// end::bad[]
