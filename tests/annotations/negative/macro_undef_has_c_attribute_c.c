// expect: frontend-error cerastes-annotation_macro_changed @ L6
// twin: tests/annotations/contracts/c_mode.c
// Mutation of the twin: `#undef __has_c_attribute` after the include, in a C file.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#undef __has_c_attribute

int abs_val(int x) CERASTES_REQUIRES(x > -1000) CERASTES_ENSURES(r, r >= 0);
