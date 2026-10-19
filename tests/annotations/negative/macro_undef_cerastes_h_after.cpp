// expect: frontend-error cerastes-annotation_macro_changed @ L7
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: `#undef CERASTES_H` after the include.
// Pins: plan R4c: annotation_macro_changed (a #define or #undef of a name the annotations depend on, outside the real header)
#include <cerastes.h>
#undef CERASTES_H

int half(int x) CERASTES_REQUIRES(x >= 0);
