// expect: frontend-error cerastes-annotation_macro_changed @ L6
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define requires requires` before the include.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#define requires requires
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
