// expect: frontend-error cerastes-annotation_macro_changed @ L6
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#define CERASTES_H` before the include: the header is skipped (the body does not use its macros).
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#define CERASTES_H
#include <cerastes.h>

int half(int x);
