// expect: frontend-error cerastes-annotation_macro_changed @ L7
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a new CERASTES_* name is defined after the include.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#define CERASTES_EXTRA 1

int half(int x) CERASTES_REQUIRES(x >= 0);
