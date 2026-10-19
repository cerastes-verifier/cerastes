// expect: frontend-error cerastes-annotation_macro_changed @ L6
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#undef __has_cpp_attribute` after the include.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#undef __has_cpp_attribute

int half(int x) CERASTES_REQUIRES(x >= 0);
