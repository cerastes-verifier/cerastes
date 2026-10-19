// expect: frontend-error cerastes-annotation_macro_changed @ L6
// A macro of the same name as an annotation would change what the annotations say.
// tag::bad[]
#include <cerastes.h>

#define ensures requires // <- error

int half(int x) CERASTES_REQUIRES(x >= 0);
// end::bad[]
