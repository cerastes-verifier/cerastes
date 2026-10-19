// expect: frontend-error cerastes-annotation_macro_changed @ L7
// expect: frontend-warning predef_macro_redefined @ L7
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define __has_cpp_attribute(x) 0` after the include.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#define __has_cpp_attribute(x) 0

int half(int x) CERASTES_REQUIRES(x >= 0);
